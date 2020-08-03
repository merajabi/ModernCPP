#ifndef _ModernCPP_SmartGuard_H_
#define _ModernCPP_SmartGuard_H_

#include "typetraits.h"

#include <assert.h>

namespace ModernCPP {
	template<typename T>
	class SmartGuard {
		mutable T *xPtr;

		public:
			SmartGuard(T* xp = nullptr){
				xPtr = xp;
			}
			SmartGuard(const SmartGuard &pg){ // its move constructor
				xPtr = pg.xPtr;
				pg.xPtr = nullptr;
			}
			SmartGuard& operator= (const SmartGuard &pg){ //  its move assignment operator
				assert(xPtr == nullptr);
				xPtr = pg.xPtr;
				pg.xPtr = nullptr;
				return *this;
			}
			~SmartGuard() {
				if(xPtr != nullptr){
					delete xPtr;
				}
			}
			T* operator* () const {
				assert(xPtr != nullptr);
				return xPtr;
			}
			T* operator ->() const {
				assert(xPtr != nullptr);
				return xPtr;
			}
	};
}
#endif //_ModernCPP_SmartGuard_H_

