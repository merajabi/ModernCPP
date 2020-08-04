#include "ModernCPP.h"

#include <iostream>
#include <string>

struct MovableClass {
	int* data;

	MovableClass():data(0){
		std::cout << "default" << std::endl;
	}
	MovableClass(const int& d){
		std::cout << "param" << std::endl;
		data = new int(d);
	}
	~MovableClass(){
		delete data;
	}

	MovableClass(const MovableClass& mv){
		std::cout << "Copy" << std::endl;
		data = new int(*mv.data);
	}
	MovableClass& operator=(const MovableClass& mv){
		std::cout << "Copy=" << std::endl;
		if (!data) {
			data = new int();
		}

		*data = *mv.data; 
		return *this;
	}

	MovableClass(const right_reference<MovableClass>& mv){
		std::cout << "Move" << std::endl;
		data = mv->data;
		mv->data = 0;
	}
	MovableClass& operator=(const right_reference<MovableClass>& mv){
		std::cout << "Move=" << std::endl;
		if (data) {
			delete data;
		}
		data = mv->data;
		mv->data = 0;
		return *this;
	}

	int get() const {
		if(!data){
			return -1;
		}
		return *data;
	}
};

MovableClass func(int x){
	return MovableClass(x);
}
MovableClass func2(int x){
	MovableClass mv(x);
	return mv;
}

right_reference<MovableClass> func3(int x){
	MovableClass mv(x);
	return refmove(mv);
}

int main(){
	{
		std::cout << "Test 1" << std::endl;
		MovableClass mv(10);
		std::cout << mv.get() << std::endl;

		MovableClass mv2(mv);

		std::cout << mv.get() << std::endl;
		std::cout << mv2.get() << std::endl;
	}
	{
		std::cout << "Test 2" << std::endl;
		MovableClass mv(10);
		std::cout << mv.get() << std::endl;

		MovableClass mv2;
		mv2 = mv;

		std::cout << mv.get() << std::endl;
		std::cout << mv2.get() << std::endl;
	}

	{
		std::cout << "Test 3" << std::endl;
		MovableClass mv(10);
		std::cout << mv.get() << std::endl;

		MovableClass mv2(refmove(mv));

		std::cout << mv.get() << std::endl;
		std::cout << mv2.get() << std::endl;
	}

	{
		std::cout << "Test 4" << std::endl;
		MovableClass mv(10);
		std::cout << mv.get() << std::endl;

		MovableClass mv2;

		mv2 = refmove(mv);
		std::cout << mv.get() << std::endl;
		std::cout << mv2.get() << std::endl;
	}

	{
		std::cout << "Test 5" << std::endl;
		MovableClass mv1 = func(20);
		std::cout << mv1.get() << std::endl;

		MovableClass mv2;
		mv2 = func(20);
		std::cout << mv2.get() << std::endl;
	}
	{
		std::cout << "Test 6" << std::endl;
		MovableClass mv1 = func2(20);
		std::cout << mv1.get() << std::endl;

		MovableClass mv2;
		mv2 = func2(20);
		std::cout << mv2.get() << std::endl;
	}

	{
		/*
		std::cout << "Test 7" << std::endl;
		MovableClass mv1 = func3(20);
		std::cout << mv1.get() << std::endl;

		MovableClass mv2;
		mv2 = func3(20);
		std::cout << mv2.get() << std::endl;
		*/
	}

	{
		std::cout << "Test 6" << std::endl;
		const MovableClass mv(10);
		std::cout << mv.get() << std::endl;

		MovableClass mv2;

		// this must give us error
		//mv2 = refmove(mv);

		std::cout << mv.get() << std::endl;
		std::cout << mv2.get() << std::endl;
	}

}

