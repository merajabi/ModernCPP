#include <iostream>

#if (__cplusplus < 201103L)
	#ifndef nullptr
		#define nullptr NULL
	#endif
	#include "thread.h"
	#include "mutex.h"
	#include "atomic.h"
	#include "smartguard.h"
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


void incref(int& x){
	x++;
}

void inccon(const int& x){
	x;
}

void inc(unsigned long x){
	std::cout << "X: " << x <<std::endl;
	for(unsigned long i=0;i<x;i++){
		counter++;
	    std::cout<<counter<<std::endl;
	}
}

class Test {
	unsigned long max;
	public:
	Test(unsigned long x=10):max(x){}
	void operator () (){
		inc(max);
	}
	void run (){
		inc(max);
	}
};

int main() {
	{
		
		thread t1(inc,10);
		t1.join();
		//void (*fp)(unsigned long) = inc;
		//thread t2(fp,10);
		thread t2(inc,10);

		Test obj1;
		thread t3(obj1);
 
		t3.join();
		t2.join();

	    std::cout<<counter<<std::endl;
		
	}
	{
		/*
		counter=0;
		thread t4((Test())); //this is to avoid what's known as C++'s most vexing parse: 
							// without the parentheses, the declaration is taken to be a declaration of a function called t4 
	    thread t5=thread(Test());

	    thread t6(Test(10));

		t6.join();
		t5.join();
		t4.join();
	    std::cout<<counter<<std::endl;
		*/
	}
	{
		/*
		counter=0;
		Test obj2;
	    thread t7(&Test::run,&obj2);

		t7.join();
	    std::cout<<counter<<std::endl;
		*/
	}
	{
		/*
		counter=0;
		SmartGuard<Test> p(new Test);
		thread t(&Test::run,p);
		t.join();
	    std::cout<<counter<<std::endl;
		*/
	}
	{
		/*
		int x=10;
		thread t(incref,std::ref(x));
		t.join();
		std::cout<<x<<std::endl;
		*/
	}
	{
		thread t(inccon,10);
		t.join();
	}
}

