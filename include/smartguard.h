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
		mutable T *xPtr;
			SmartGuard(SmartGuard &pg);
			//SmartGuard(const SmartGuard &pg);
			SmartGuard& operator= (SmartGuard &pg);
			SmartGuard& operator= (const SmartGuard &pg);
		public:
			SmartGuard(T *xp=nullptr){
				xPtr=xp;
			}
			SmartGuard(const SmartGuard &pg){
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
			}
			SmartGuard& operator= (T *xp){
				assert(xPtr==nullptr);
				xPtr=xp;
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
			T* move(){
				T* tmp=xPtr;
				xPtr=nullptr;
				return tmp;
			}
			T* release(){
				T* tmp=xPtr;
				xPtr=nullptr;
				return tmp;
			}
			void reset(){
				if(xPtr!=nullptr){
					delete xPtr;
				}
				xPtr=nullptr;
			}
	};


//	template<typename T>
//	typedef SmartGuard<typename T> unique_ptr<typename T>;
/*	struct Vector
	{
		typedef Matrix<N, 1> type;
	};
*/
	template<typename T>
	T* move (SmartGuard<T>& p){
		return p.move();
	}
};
#endif //_ModernCPP_SmartGuard_H_

