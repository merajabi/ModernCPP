#include <iostream>
#include "type_traits.h"

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
*/
using namespace ModernCPP;


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

/*
template<typename ParamType>
struct ThreadData {
	ParamType& param;
	ThreadData(ParamType& param):param(param){};
	void run(){param++;}
};

template < typename ParamType >
struct ThreadData < typename add_const<ParamType>::value > {
	const ParamType& param;
	ThreadData(const ParamType& param):param(param){};
	void run(){param;}
};
*/

int main () {
	int y=10;
	ThreadData<int> x(y);
	std::cout << x.param << std::endl ;
	x.run();
	std::cout << x.param << std::endl ;

	ThreadData<const int> z(10);
	std::cout << z.param << std::endl ;

/*
	remap<int> test2;
	std::cout << test2.foo(50) << std::endl ;


	remap<unsigned char> test;
	std::cout << test.foo(50) << std::endl ;
*/
	return 0;
}
