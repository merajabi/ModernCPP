#include <thread>
#include <iostream>
#include <mutex>
#include "atomic.h"

std::mutex m;
ModernCPP::Atomic<unsigned long> counter(0);

void inc() {
	while(1){
		unsigned long tmp = counter.Load();
		if(tmp<1000000ul) {
			std::lock_guard<std::mutex> lk(m);
			tmp = counter.Load();
			if(tmp<1000000ul){
				tmp++;
				counter.Store(tmp);
			}
		}else{
			break;
		}
	}
}

int main() {
    std::thread t1(inc);
    std::thread t2(inc);
    std::thread t3(inc);
    std::thread t4(inc);
    std::thread t5(inc);

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();

    std::cout<<counter<<std::endl;
}

