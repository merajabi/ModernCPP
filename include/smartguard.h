#include <iostream>
#include <assert.h>
#define nullptr 0
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
	typedef SmartGuard unique_ptr;
};
