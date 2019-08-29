#include <iostream>

#if (__cplusplus < 201103L)
	#ifndef nullptr
		#define nullptr NULL
	#endif
	#include "thread.h"
	#include "mutex.h"
	#include "atomic.h"
	using namespace ModernCPP;
#else
	#include <thread>
	#include <mutex>
	#include <atomic>
	using namespace std;
#endif 

mutex m;
atomic<unsigned long> counter(10000ul);

int inc(const std::string& x) {
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

class Test {
	public:
	void operator () (){
		inc("1");
	}
};

int main() {
	Test obj;
	int (*fp)(const std::string&) = inc;
	{
		thread t1(obj);
		thread t2(inc,"5");
		thread t3(fp,"5");
		t3.join();
		t2.join();
		t1.join();
	}
    std::cout<<counter<<std::endl;
}

