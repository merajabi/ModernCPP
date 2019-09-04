# ModernCPP
Light weight Modern CPP interface to simulate C++11 standard for old C++98 compilers
# Samples
## Note: you should add these lines in the begining of your code

```cpp
#if (__cplusplus < 201103L)
	#include "type_traits.h"
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

```

## Example 1: running a simple function on another thread
```cpp
void func() {
    std::cout<<"hello"<<std::endl;
}

int main()
{
    thread t(func);
	t.join();
}
```
## Example 2: running three functor each on one thread, adding to a counter using lock
```cpp
mutex m;
unsigned long counter(0);

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void operator () (){
		for(unsigned long i=0;i<x;i++){
			lock_guard<mutex> lk(m);
			counter++;					// this should be atomic or we should aquire lock
		}
	}
	unsigned long Get() const {return x;}
};

int main () {
	thread t4((Sample())); //this is to avoid what's known as C++'s most vexing parse: 
							// without the parentheses, the declaration is taken to be a declaration of a function called t4 
	thread t5=thread(Sample());

	thread t6(Sample(10));

	t6.join();
	t5.join();
	t4.join();
	std::cout<<counter<<std::endl;
	return 0;
}
```
## Example 3: running three functor each on one thread, adding to a atomic counter
```cpp
atomic<unsigned long> counter(0);

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void operator () (){
		for(unsigned long i=0;i<x;i++){
			counter++;					// this should be atomic or we should aquire lock
		}
	}
	unsigned long Get() const {return x;}
};

int main () {
	thread t4((Sample())); //this is to avoid what's known as C++'s most vexing parse: 
							// without the parentheses, the declaration is taken to be a declaration of a function called t4 
	thread t5=thread(Sample());

	thread t6(Sample(10));

	t6.join();
	t5.join();
	t4.join();
	std::cout<<counter<<std::endl;
	return 0;
}
```
## Example 4: passing argument to thread function
```cpp
void greeting(std::string const& message) {
    std::cout<<message<<std::endl;
}

int main() {
    thread t1(bind(greeting,"hi!"));
    thread t2(greeting,"hi!");
    t1.join();
    t2.join();
	return 0;
}
```

## Example 5: passing reference to thread 
```cpp

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void Inc (){ x+=20; }
	unsigned long Get() const {return x;}
};

void incref(int& x){
	x++;
}
void incSampleRef(Sample& obj){
	obj.Inc();
}

int main () {
	int x=10;
	Sample obj;
    std::cout<< x <<std::endl;
    std::cout<< obj.Get() <<std::endl;
	thread t1(incref,ref(x));
	thread t2(incSampleRef,ref(obj));
	t2.join();
	t1.join();
    std::cout<< x <<std::endl;
    std::cout<< obj.Get() <<std::endl;
	return 0;
}

```
## Example 6: Invoking a member function on a new thread
```cpp
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

```

## compile & test
compile with std=c++11 or std=c++98 alike :)

```sh
# g++ -std=c++98 -Iinclude -pthread test/basic.cpp
or
# g++ -std=c++11 -Iinclude -pthread test/basic.cpp
```
