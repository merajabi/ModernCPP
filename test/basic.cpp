#include <iostream>
#include <vector>
#include <assert.h>

#if (__cplusplus < 201103L)
	#include "thread.h"
	#include "mutex.h"
	#include "atomic.h"
	#include "unique_ptr.h"
	using namespace ModernCPP;
#else
	#include <thread>
	#include <mutex>
	#include <atomic>
	#include <functional> //for std::bind
	using namespace std;
#endif 

const unsigned long max=10000ul;
mutex m;
atomic<unsigned long> counter(0);


/*
int dec(const std::string& x) {
	while(1){
		unsigned long tmp = counter.load();
		if(tmp>0) {
			lock_guard<mutex> lk(m);
			tmp = counter.load();
			if(tmp>0){
				tmp--;
				counter.store(tmp);
			}
		} else {
			break;
		}
	}
	return 0;
}
*/

void inc(long x){
	for(long i=0;i<x;i++){
		counter++;
	}
}

void incconref(const int& x){
	for(int i=0;i<x;i++){
		counter++;
	}
}

void incref(int& x){
	lock_guard<mutex> lk(m);
	x++;
}


class Sample {
	long x;
	public:
	Sample(long x=10):x(x){}
	void operator () (){
		for(long i=0;i<x;i++){
			counter++;
		}
	}
	void Run (){
		x+=20;
	}
	long Get() const {return x;}
};

void incSampleRef(Sample& obj){
	lock_guard<mutex> lk(m);
	obj.Run();
}

void incSample(Sample x){
	for(long i=0;i<x.Get();i++){
		counter++;
	}
}

void incSampleRefConst(const Sample& x){
	for(long i=0;i<x.Get();i++){
		counter++;
	}
}

std::vector<SmartGuard<thread> > createList(){
	std::vector<SmartGuard<thread> > tvec;
	for(int i=0; i<3; i++){
		Sample obj(10);
		SmartGuard<thread> tg ( new thread(obj) );
		tvec.push_back(refmove(tg));

		//tvec.push_back(new thread(obj));
	}
	return tvec;
}
void createList(std::vector<SmartGuard<thread> >& tvec){
	for(int i=0; i<3; i++){
		Sample obj(10);
		//SmartGuard<thread> tg ( new thread(obj) );
		//tvec.push_back(refmove(tg));

		tvec.push_back(new thread(obj));
	}
}
void waitList(const std::vector<SmartGuard<thread> >& tvec){
	for(int i=0; i<3; i++){
		tvec[i]->join();
	}
}

int main() {
	{	// test 1
	    std::cout<<" test 1"<<std::endl;
		int x=10;
		counter=0;
		thread t1(inc,x);
		thread t2(inc,x);
		t1.join();
		t2.join();
		assert(counter.load()==20);
	    std::cout<<counter<<std::endl;
	}
	{//test 2
	    std::cout<<" test 2"<<std::endl;
		counter=0;
		thread t1(inc,10);
		thread t2(inc,10);
		t1.join();
		t2.join();
		assert(counter.load()==20);
	    std::cout<<counter<<std::endl;
	}

	{// test 3
	    std::cout<<" test 3"<<std::endl;
		int x=10;
	    std::cout<< x <<std::endl;
		thread t1(incref,ref(x));
		thread t2(incref,ref(x));
		t2.join();
		t1.join();
		assert(x==12);
	    std::cout<< x <<std::endl;
	}


	{ // test 4
	    std::cout<<" test 4"<<std::endl;
		const int x=10;
	    std::cout<< x <<std::endl;
		thread t1(incconref,x);
		thread t2(incconref,x);
		t2.join();
		t1.join();
		assert(x==10);
	    std::cout<< x <<std::endl;
	}

	{	//test 5
	    std::cout<<" test 5"<<std::endl;
		counter=0;
		void (*fp)(long) = inc;
		thread t(fp,10);
		t.join();
		assert(counter.load()==10);
	    std::cout<<counter<<std::endl;
	}


	{	//test 6
	    std::cout<<" test 6"<<std::endl;
		Sample obj;
		counter=0;
		thread t1(incSample,obj);
		thread t2(incSample,obj);
		t1.join();
		t2.join();
		assert(counter.load()==20);
	    std::cout<<counter<<std::endl;
	}
	{	//test 7
	    std::cout<<" test 7"<<std::endl;
		counter=0;
		thread t1(incSample,Sample());
		thread t2(incSample,Sample());
		t1.join();
		t2.join();
		assert(counter.load()==20);
	    std::cout<<counter<<std::endl;
	}

	{	//test 8
	    std::cout<<" test 8"<<std::endl;
		Sample obj;
	    std::cout<<obj.Get()<<std::endl;
		thread t1(incSampleRef,ref(obj));
		thread t2(incSampleRef,ref(obj));
		t1.join();
		t2.join();
		assert(obj.Get()==50);
	    std::cout<<obj.Get()<<std::endl;
	}

	{	//test 9
	    std::cout<<" test 9"<<std::endl;
		const Sample obj;
		counter=0;
		thread t1(incSampleRefConst,obj);
		thread t2(incSampleRefConst,obj);
		t1.join();
		t2.join();
		assert(counter.load()==20);
	    std::cout<<counter<<std::endl;
	}

	{	//test 10
	    std::cout<<" test 10"<<std::endl;
		Sample obj;
		counter=0;
		thread t1(obj);
 		t1.join();
		assert(counter.load()==10);
	    std::cout<<counter<<std::endl;
	}

	{	// test 11
	    std::cout<<" test 11"<<std::endl;		
		counter=0;
		thread t4((Sample())); //this is to avoid what's known as C++'s most vexing parse: 
							// without the parentheses, the declaration is taken to be a declaration of a function called t4 

	    // thread t5 = thread(Sample()); // can not copy construct a thread

	    thread t6(Sample(10));

		t6.join();
		//t5.join();
		t4.join();
		assert(counter.load()==20);
	    std::cout<<counter<<std::endl;
		
	}
	{	//test 12
	    std::cout<<" test 12"<<std::endl;		
		Sample obj;
	    std::cout<<obj.Get()<<std::endl;
		// AUTO( f1 , bind(&Sample::Run, &obj) );
	    thread t(bind(&Sample::Run, &obj));
		t.join();
	    std::cout<<obj.Get()<<std::endl;
		
	}
	{
		//test 13
		unique_ptr<Sample> p(new Sample);
	    std::cout<<p->Get()<<std::endl;
		thread t(bind(&Sample::Run,p.get()));
		t.join();
	    std::cout<<p->Get()<<std::endl;		
	}

	{
		//test 14
		std::vector<SmartGuard<thread> > tvec;
		counter=0;
		for(int i=0; i<3; i++){
			//Sample obj(10);
			//SmartGuard<thread> tg ( new thread(obj) );
			//tvec.push_back(refmove(tg));

			tvec.push_back(new thread(Sample(10)));
		}
		for(int i=0; i<3; i++){
			tvec[i]->join();
		}
	    std::cout<<counter<<std::endl;
	}
	{
		//test 14
		counter=0;
		std::vector<SmartGuard<thread> > tvec1;
		createList(tvec1);
		std::vector<SmartGuard<thread> > tvec2 = createList();

		waitList(tvec2);
		waitList(tvec1);
	    std::cout<<counter<<std::endl;
	}

}

