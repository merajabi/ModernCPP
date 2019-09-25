#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
//for std::bind
#include <functional>

using namespace std;

atomic<unsigned long> counter(0);

class Sample {
	unsigned long x;
	public:
	Sample(unsigned long x=10):x(x){}
	void operator () (){
		for(unsigned long i=0;i<x;i++){
			counter++;
		}
	}
	unsigned long Get() const {return x;}
};

void call(unique_ptr<Sample> p){
	(*p)();
}

int main() {
	{
		//test 13
	    std::cout<<counter<<std::endl;

		unique_ptr<Sample> p(new Sample);

		std::thread t(call,std::move(p));
		t.join();

	    std::cout<<counter<<std::endl;		
	}
}

