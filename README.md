# ModernCPP
Light weight Modern CPP interface to simulate C++11 standard for old C++98 compilers
# Samples
##Note: you should add these lines in the begining of your code

```cpp
#if (__cplusplus < 201103L)
	#include "type_traits.h"
	#include "thread.h"
	#include "mutex.h"
	#include "atomic.h"
	#include "unique_ptr.h"
	using namespace ModernCPP;
	#define AUTO(x,y) autotypeof<__typeof__(y)>::type x = y
#else
	#include <thread>
	#include <mutex>
	#include <atomic>
	#include <functional>
	using namespace std;
	#define AUTO(x,y) auto x = y
#endif 
```

##example1: running a simple function on another thread
```cpp
#include <thread>
#include <iostream>

void func() {
    std::cout<<"hello"<<std::endl;
}

int main()
{
    thread t(func);
}
```

## compile & test
compile with std=c++11 or std=c++98 alike :)

```sh
# g++ -std=c++98 -Iinclude -pthread test/basic.cpp
or
# g++ -std=c++11 -Iinclude -pthread test/basic.cpp
```
