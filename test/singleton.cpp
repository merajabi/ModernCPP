#include <iostream>
#include <vector>

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

class SafeSingle {
		static atomic<SafeSingle*> instance;	//comment this line or next one to switch between atomic and nonatomic instance
		//static SafeSingle* instance;

		static atomic<long> call_counter;
		static atomic<long> lock_counter;
		static mutex m;

		atomic<long> local_counter;

		SafeSingle():local_counter(0){};
		~SafeSingle(){};
	  
	public:
		static SafeSingle* GetInstance(){
			if(instance==0) {
				lock_guard<mutex> lk(m);		// comment this line to see how the result will change when there is flaw in the code!!!
				if(instance==0){
					SafeSingle* tmp = new SafeSingle();
					instance=tmp;
				}
				lock_counter++;
			}
			return instance;
		};
		static void DestroyInstance(){
			lock_guard<mutex> lk(m);
			if(instance!=0){
				delete (SafeSingle*)instance;
				instance=0;
			}
		};

		static long GetCallCount(){
			return call_counter;
		}
		static long GetLockCount(){
			return lock_counter;
		}

		void Operate(){
			++local_counter;
			++call_counter;
		}

		long GetLocal(){
			return local_counter;
		}

};

mutex SafeSingle::m;
atomic<long> SafeSingle::call_counter(0);
atomic<long> SafeSingle::lock_counter(0);

atomic<SafeSingle*> SafeSingle::instance(0);	//comment this line or next one to switch between atomic and nonatomic instance
//SafeSingle* SafeSingle::instance(0);

void inc(long x){
	SafeSingle* safePtr=SafeSingle::GetInstance();
	for(long i=1;i<=x;i++){
		safePtr->Operate();
	}
}

const int numTry=10000;
const int numThread=5;

int main() {
	long counter=0;
	for (int i=0;i<numTry;i++){
		std::vector<thread> tv;
		for(int j=0;j<numThread;j++){
			tv.push_back(thread(inc,10));
		}
		for(int j=0;j<tv.size();j++){
			tv[j].join();
		}
		counter+=SafeSingle::GetInstance()->GetLocal();
		SafeSingle::DestroyInstance();
	}
	std::cout << counter << std::endl;						// if we have flaw in singleton class, this counter will be lower than call_counter
	std::cout << SafeSingle::GetCallCount() << std::endl;
	std::cout << SafeSingle::GetLockCount() << std::endl;	// I expect the (lock_counter-numTry) to be smaller when I use atomic instance: but I have not seen that !!!
	return 0;
}

