#ifndef _ModernCPP_ModernCPP_H_
#define _ModernCPP_ModernCPP_H_
	#if (__cplusplus < 201103L)
		#include "typetraits.h"
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
		#include <functional> //for std::bind
		using namespace std;
		#define AUTO(x,y) auto x = y
	#endif 
#endif //_ModernCPP_ModernCPP_H_

