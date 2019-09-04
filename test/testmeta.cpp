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
/*
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
*/
/*
struct MyClass {
	int x;
    int DoStuff(double d){x+=d;}; // actually a DoStuff(MyClass* this, double d)
};
int main () {
	MyClass obj;
	obj.x=0;
	//std::function<int(double d)> f = std::bind(&MyClass::DoStuff, &obj, std::placeholders::_1);
	auto f = std::bind(&MyClass::DoStuff, &obj, std::placeholders::_1) ; // auto f = std::bind(...); in C++11

	f(5);
	std::cout << obj.x << std::endl ;
	return 0;
}
*/
/*
void inc(int& x){
	x++;
}

int main () {
	int x=10;

	AUTO( f , bind(inc,ref(x)) );
	f();

	std::cout << x << std::endl ;
	return 0;
}
*/

/*
template<typename F,typename U,typename P>
void func(F f,U u,P p){
	(u->*f)(p);
	return;
}

class Sample {
	int x;
	public:
	Sample(int x=10):x(x){}
	void operator () (){
	}
	void Add (int t){
		x+=t;
	}
	unsigned long Get() const {return x;}
};

int main () {
	Sample obj;
	int t=5;

	func(&Sample::Add,&obj,t);
	
	std::cout << obj.Get() << std::endl ;
	return 0;
}
*/

/*
template <typename T> struct proxy;

template <typename T, typename R, typename ...Args>
struct proxy<R (T::*)(Args...)>
{
    static R call(T & obj, R (T::*mf)(Args...),Args &&... args)
    {
        return (obj.*mf)(std::forward<Args>(args)...);
    }
};


template <typename T, typename R, typename ...Args>
R proxycall(T & obj, R (T::*mf)(Args...), Args &&... args)
{
    return (obj.*mf)(std::forward<Args>(args)...);
}

class hello{
public:
    void f(){
        cout<<"f"<<endl;
    }
    virtual void ff(){
        cout<<"ff"<<endl;
    }
};

int main () {
	hello obj;

	proxycall(obj, &hello::ff);

	proxy<void(hello::*)()>::call(obj,&hello::ff);

	// or

	typedef proxy<void(hello::*)()> hello_proxy;
	hello_proxy::call(obj, &hello::ff);

};
*/

/*
template <typename T> struct Delegate;
template <typename T, typename R, typename ...Args>
class Delegate<R (T::*)(Args...)> {
    T obj;
    R (T::*func)(Args...) ;

public:
    Delegate(R (T::*mf)(Args...),T obj) : obj(obj), func(mf){
    }

    R operator()(Args &&... args) {
        return (obj.*func)(std::forward<Args>(args)...);
    }

private:

    Delegate();
    Delegate( const Delegate& );
};


class A {
public:
    virtual void Fn( int i ) {
		std::cout << i << std::endl;
    }
};

int main() {
    A a;
    Delegate<void(A::*)(int)> cbk(&A::Fn, a);
    cbk( 3 );
}
*/
/*
template <typename T> struct Delegate;

template <typename T, typename R, typename P>
class Delegate<R (T::*)(P)> {
    T* obj;
    R (T::*func)(P) ;

public:
    Delegate(R (T::*mf)(P),T* obj) : obj(obj), func(mf){
    }

    R operator()(P p) {
        return (obj->*func)(p);
    }

private:

    Delegate();
    Delegate( const Delegate& );
};

class A {
	int x;
public:
	A():x(0){};
    virtual void Fn( int i ) {
		x+=i;
		std::cout << x << std::endl;
    }
	int Get(){return x;};
};

int main() {
    A a;
    Delegate<void(A::*)(int)> cbk(&A::Fn, &a);
    cbk( 3 );
	std::cout << a.Get() << std::endl;
}
*/
/*
template <typename T> struct Delegate;

template <typename T, typename R, typename P>
class Delegate<R (T::*)(P)> {
    T* obj;
    R (T::*func)(P) ;

public:
    Delegate(R (T::*mf)(P),T* obj) : obj(obj), func(mf){
    }

    R operator()(P p) {
        return (obj->*func)(p);
    }

private:

    Delegate();
    //Delegate( const Delegate& );
};

template <typename T, typename R, typename P>
Delegate<R (T::*)(P)> proxycall(R (T::*mf)(P),T* obj) {
    //return (obj.*mf)(std::forward<Args>(args)...);
    return Delegate<R (T::*)(P)>(mf, obj);
}

class A {
	int x;
public:
	A():x(0){};
    virtual void Fn( int i ) {
		x+=i;
		std::cout << x << std::endl;
    }
	int Get(){return x;};
};

int main() {
    A a;
    Delegate<void(A::*)(int)> cbk(&A::Fn, &a);
    cbk( 3 );
	std::cout << a.Get() << std::endl;

	AUTO( f , proxycall(&A::Fn, &a) );
	f(5);
	std::cout << a.Get() << std::endl;
}
*/

class A {
	int x;
public:
	A():x(0){};
    virtual void Fn( int i ) {
		x+=i;
    }
	int Get(){return x;};
};

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void Run (void){
		x+=20;
	}
	unsigned long Get() const {return x;}
};

void inc(int& x){
	x++;
}

void hello(){
	std::cout << "hello" << std::endl;
}

int main() {

	{

		A a;
		std::cout << a.Get() << std::endl;
		AUTO( f1 , bind(&A::Fn, &a) );
		f1(5);
		std::cout << a.Get() << std::endl;

	}
	{

		int x=10;
		//void (&fp)(int&) = inc;
		AUTO( f2 , bind(inc,ref(x)) );
		f2();
		std::cout << x << std::endl ;

	}
	{

		Sample obj;
		std::cout << obj.Get() << std::endl;
		AUTO( f3 , bind(&Sample::Run, &obj) );
		f3();
		std::cout << obj.Get() << std::endl;

	}
	{

		//void (&fp)(int&) = inc;
		AUTO( f4 , bind(hello) );
		f4();

	}
	{
		unique_ptr<Sample> p(new Sample);
	    std::cout<<p->Get()<<std::endl;
		AUTO( f5 , bind(&Sample::Run,(Sample*)p) );
		f5();
	    std::cout<<p->Get()<<std::endl;		
	}

}

/*
class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void Run (void){
		x+=20;
	}
	unsigned long Get() const {return x;}
};
void dosomething1(unique_ptr<Sample>& x){
	std::cout<<x->Get()<<std::endl;		
}
void dosomething2(unique_ptr<Sample> x){
	std::cout<<x->Get()<<std::endl;		
}
unique_ptr<Sample> dosomething3(unique_ptr<Sample> x){
	std::cout<<x->Get()<<std::endl;		
	return move(x);
}
int main () {
	unique_ptr<Sample> x(new Sample);
	Sample *xPtr = move(x);
	unique_ptr<Sample> y(xPtr);
	dosomething1(y);

	unique_ptr<Sample> w = dosomething3(move(y));

	unique_ptr<Sample> z=move(w);
	dosomething2(move(z));
}
*/
