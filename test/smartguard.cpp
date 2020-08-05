#include "ModernCPP.h"

#include <iostream>
#include <string>

struct Sample {
	int x;
	Sample():x(0){}
	Sample(int x):x(x){}
	void print() const {
		std::cout << x << std::endl;
	}
};


Sample* func(int x){
	SmartGuard<Sample> mv(new Sample(x));
	return mv.release();
}
/*
right_reference<SmartGuard> func3(int x){
	SmartGuard mv(x);
	return refmove(mv);
}
*/
int main(){
	{
		std::cout << "Test 1" << std::endl;
		SmartGuard<Sample> mv(new Sample(10));
		SmartGuard<Sample> mv2(refmove(mv));

		if(mv){
			mv->print();
		}
		mv2->print();
	}

	{
		std::cout << "Test 2" << std::endl;
		SmartGuard<Sample> mv(new Sample(15));
		SmartGuard<Sample> mv2;

		mv2 = refmove(mv);

		if(mv){
			mv->print();
		}
		mv2->print();
	}
	{
		std::cout << "Test 3" << std::endl;
		SmartGuard<Sample> mv( func(20) );
		mv->print();
	}
	{
		std::cout << "Test 4" << std::endl;
		SmartGuard<Sample> mv;
		mv = func(25);
		mv->print();
	}
	return 0;
}
