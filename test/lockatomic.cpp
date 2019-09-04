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


long free_counter(0);

void inc(long x){
	for(long i=0;i<x;i++){
		free_counter++;
	}
}

mutex m;
long locked_counter(0);

void inc_lock(long x){
	for(long i=0;i<x;i++){
		lock_guard<mutex> lk(m);
		locked_counter++;
	}
}

atomic<long> atomic_counter(0);

void inc_atomic(long x){
	for(long i=0;i<x;i++){
		atomic_counter++;
	}
}

int main() {
	{
		thread t1(inc,100000);
		thread t2(inc,100000);
		t2.join();
		t1.join();
	}
	{
		thread t1(inc_lock,100000);
		thread t2(inc_lock,100000);
		t2.join();
		t1.join();
	}
	{
		thread t1(inc_atomic,100000);
		thread t2(inc_atomic,100000);
		t2.join();
		t1.join();
	}

	std::cout << free_counter << std::endl;
	std::cout << locked_counter << std::endl;
	std::cout << atomic_counter << std::endl;
}
