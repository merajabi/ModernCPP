#if (__cplusplus < 201103L)
	#include "typetraits.h"
	#include "thread.h"
	#include "mutex.h"
	#include "atomic.h"
	#include "unique_ptr.h"
	using namespace ModernCPP;
	#define AUTO(x,y) autotypeof<__typeof__(y)>::type x = y
#else
	#include <thread>
	#include <mutex>
	#include <atomic>
	#include <functional>
	using namespace std;
	#define AUTO(x,y) auto x = y
#endif 

#include <iostream>

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void Inc (){ x+=20; }
	unsigned long Get() const {return x;}
};
int main () {
	{
		Sample obj;
		std::cout<<obj.Get()<<std::endl;
		thread t(bind(&Sample::Inc, &obj));
		t.join();
		std::cout<<obj.Get()<<std::endl;
	}
	{
		unique_ptr<Sample> p(new Sample);
	    std::cout<<p->Get()<<std::endl;
		thread t(bind(&Sample::Inc,p.get()));
		t.join();
	    std::cout<<p->Get()<<std::endl;
	}
	return 0;
}

