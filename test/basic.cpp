#include <iostream>

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

void inc(unsigned long x){
	for(unsigned long i=0;i<x;i++){
		counter++;
	}
}

void incconref(const int& x){
	for(unsigned long i=0;i<x;i++){
		counter++;
	}
}

void incref(int& x){
	lock_guard<mutex> lk(m);
	x++;
}


class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void operator () (){
		for(unsigned long i=0;i<x;i++){
			counter++;
		}
	}
	void Run (){
		x+=20;
	}
	unsigned long Get() const {return x;}
};

void incSampleRef(Sample& obj){
	lock_guard<mutex> lk(m);
	obj.Run();
}

void incSample(Sample x){
	for(unsigned long i=0;i<x.Get();i++){
		counter++;
	}
}

void incSampleRefConst(const Sample& x){
	for(unsigned long i=0;i<x.Get();i++){
		counter++;
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
	    std::cout<<counter<<std::endl;
	}
	{//test 2
	    std::cout<<" test 2"<<std::endl;
		counter=0;
		thread t1(inc,10);
		thread t2(inc,10);
		t1.join();
		t2.join();
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
	    std::cout<< x <<std::endl;
	}

	{	//test 5
	    std::cout<<" test 5"<<std::endl;
		counter=0;
		void (*fp)(unsigned long) = inc;
		thread t(fp,10);
		t.join();
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
	    std::cout<<counter<<std::endl;
	}
	{	//test 7
	    std::cout<<" test 7"<<std::endl;
		counter=0;
		thread t1(incSample,Sample());
		thread t2(incSample,Sample());
		t1.join();
		t2.join();
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
	    std::cout<<counter<<std::endl;
	}

	{	//test 10
	    std::cout<<" test 10"<<std::endl;
		Sample obj;
		counter=0;
		thread t1(obj);
 		t1.join();
	    std::cout<<counter<<std::endl;
	}

	{	// test 11
	    std::cout<<" test 11"<<std::endl;		
		counter=0;
		thread t4((Sample())); //this is to avoid what's known as C++'s most vexing parse: 
							// without the parentheses, the declaration is taken to be a declaration of a function called t4 
	    thread t5=thread(Sample());

	    thread t6(Sample(10));

		t6.join();
		t5.join();
		t4.join();
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
		thread t(bind(&Sample::Run,(Sample*)p));
		t.join();
	    std::cout<<p->Get()<<std::endl;		
	}
}

