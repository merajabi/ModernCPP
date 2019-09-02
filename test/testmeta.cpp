#include <iostream>
#include "type_traits.h"

int main () {
	remap<int> test2;
	std::cout << test2.foo(50) << std::endl ;


	remap<unsigned char> test;
	std::cout << test.foo(50) << std::endl ;

	return 0;
}
