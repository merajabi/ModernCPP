#include "ModernCPP.h"

#include <iostream>
#include <string>

struct MovableClass {
	int* data;

	MovableClass():data(0){
	}
	MovableClass(const int& d){
		data = new int(d);
	}
	~MovableClass(){
		delete data;
	}

	MovableClass(const MovableClass& mv){
		data = new int(*mv.data);
	}
	MovableClass& operator=(const MovableClass& mv){
		if (!data) {
			data = new int();
		}
		*data = *mv.data; 
		return *this;
	}

	MovableClass(const right_reference<MovableClass>& mv){
		data = mv->data;
		mv->data = 0;
	}
	MovableClass& operator=(const right_reference<MovableClass>& mv){
		if (data) {
			delete data;
		}
		data = mv->data;
		mv->data = 0;
		return *this;
	}

	void print() const {
		if(data){
			std::cout << *data << std::endl;
		}
		else{
			std::cout << "empty" << std::endl;
		}
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
		mv.print();

		MovableClass mv2(mv);

		mv.print();
		mv2.print();
	}
	{
		std::cout << "Test 2" << std::endl;
		MovableClass mv(10);
		mv.print();

		MovableClass mv2;
		mv2 = mv;

		mv.print();
		mv2.print();
	}

	{
		std::cout << "Test 3" << std::endl;
		MovableClass mv(10);
		mv.print();

		MovableClass mv2(refmove(mv));

		mv.print();
		mv2.print();
	}

	{
		std::cout << "Test 4" << std::endl;
		MovableClass mv(10);
		mv.print();

		MovableClass mv2;

		mv2 = refmove(mv);
		mv.print();
		mv2.print();
	}

	{
		std::cout << "Test 5" << std::endl;
		MovableClass mv1 = func(20);
		mv1.print();

		MovableClass mv2;
		mv2 = func(20);
		mv2.print();
	}
	{
		std::cout << "Test 6" << std::endl;
		MovableClass mv1 = func2(20);
		mv1.print();

		MovableClass mv2;
		mv2 = func2(20);
		mv2.print();
	}

	{
		/*
		std::cout << "Test 7" << std::endl;
		MovableClass mv1 = func3(20);
		mv1.print();

		MovableClass mv2;
		mv2 = func3(20);
		mv2.print();
		*/
	}

	{
		std::cout << "Test 6" << std::endl;
		const MovableClass mv(10);
		mv.print();

		MovableClass mv2;

		// this must give us error
		//mv2 = refmove(mv);

		mv.print();
		mv2.print();
	}

}

