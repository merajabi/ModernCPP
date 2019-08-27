#include <thread>
#include <iostream>
#include "mutex.h"
#include "atomic.h"

ModernCPP::Mutex m;
ModernCPP::Atomic<unsigned long> counter(0);

void inc() {
	while(1){
		unsigned long tmp = counter.Load();
		if(tmp<1000ul) {
			m.Lock();
			tmp = counter.Load();
			if(tmp<1000ul){
				tmp++;
				counter.Store(tmp);
			}
			m.Unlock();
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

