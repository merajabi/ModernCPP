#ifndef _ModernCPP_SmartGuard_H_
#define _ModernCPP_SmartGuard_H_

#include <assert.h>
#if (__cplusplus < 201103L)
	#ifndef nullptr
		#define nullptr NULL
	#endif
#endif

namespace ModernCPP {
	template<typename T>
	class SmartGuard {
		T *xPtr;
			SmartGuard(const SmartGuard &pg);
			SmartGuard& operator= (const SmartGuard &pg);
		public:
			SmartGuard(T *xp=nullptr){
				xPtr=xp;
			}
			SmartGuard(SmartGuard &pg){
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
			}
			SmartGuard& operator= (SmartGuard &pg){
				assert(xPtr==nullptr);
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
				return *this;
			}
			~SmartGuard() {
				if(xPtr!=nullptr){
					delete xPtr;
				}
			}
	/*
			operator T() {
				assert(xPtr!=nullptr);
				return *xPtr;
			}
	*/
			operator T&() {
				assert(xPtr!=nullptr);
				return *xPtr;
			}

			T* operator ->() {
				assert(xPtr!=nullptr);
				return xPtr;
			}
			T* operator ->() const {
				assert(xPtr!=nullptr);
				return xPtr;
			}
	};
	//typedef SmartGuard unique_ptr;
};
#endif //_ModernCPP_SmartGuard_H_

