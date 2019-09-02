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

/*
template < typename T, typename dummy = T >
struct remap {
    int foo(int x){
		return x;
	}
};

template < typename T >
struct remap < T, typename enable_if< is_same<T, unsigned char>::value
                        			|| is_same<T, signed char>::value, T >::type > {
    int foo(int x){
		return x*x;
	}
};
int main () {
	remap<int> test2;
	std::cout << test2.foo(50) << std::endl ;


	remap<unsigned char> test;
	std::cout << test.foo(50) << std::endl ;
	return 0;
}
*/

/*
template<typename ParamType, typename dummy = ParamType>
struct ThreadData {
	ParamType& param;
	ThreadData(ParamType& param):param(param){};
	void run(){param++;}
};

template < typename ParamType >
struct ThreadData < ParamType, typename enable_if< is_same<ParamType, typename add_const<ParamType>::value >::value, ParamType >::type > {
	const ParamType& param;
	ThreadData(const ParamType& param):param(param){};
	void run(){param;}
};


int main () {
	int y=10;
	ThreadData<int> x(y);
	std::cout << x.param << std::endl ;
	x.run();
	std::cout << x.param << std::endl ;

	ThreadData<const int> z(10);
	std::cout << z.param << std::endl ;

	return 0;
}
*/

template<typename T,typename U>
void func(T f,U x){
	f(x);
}

void inc(int& x){
	x++;
}

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void operator () (){
	}
	void Run (){
		x+=20;
	}
	unsigned long Get() const {return x;}
};

void run(Sample& obj){
	obj.Run();
}

int main () {
	int x=10;
	std::cout << &x << std::endl ;
	std::cout << addressof(x) << std::endl ;

	std::cout << x << std::endl ;
	func(inc,x);
	std::cout << x << std::endl ;

	std::cout << x << std::endl ;
	func(inc,ref(x));
	std::cout << x << std::endl ;

	Sample obj;
	std::cout << &obj << std::endl ;
	std::cout << addressof(obj) << std::endl ;

	std::cout << obj.Get() << std::endl ;
	func(run,obj);
	std::cout << obj.Get() << std::endl ;

	std::cout << obj.Get() << std::endl ;
	func(run,ref(obj));
	std::cout << obj.Get() << std::endl ;
	return 0;
}

