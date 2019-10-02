#ifndef _ModernCPP_unique_ptr_H_
#define _ModernCPP_unique_ptr_H_

#include <assert.h>
#include "type_traits.h"

namespace ModernCPP {
	template<typename T>
	class unique_ptr {
		mutable T *xPtr;
			unique_ptr(unique_ptr &pg);
			unique_ptr& operator= (unique_ptr &pg);
		public:
			unique_ptr(T *xp=nullptr){
				xPtr=xp;
			}
			unique_ptr& operator= (T *xp){
				assert(xPtr==nullptr);
				xPtr=xp;
				return *this;
			}
			unique_ptr(const unique_ptr &pg){
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
			}
			unique_ptr& operator= (const unique_ptr &pg){
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
				return *this;
			}
			template<typename U>
			unique_ptr(const unique_ptr<U> &pg){
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
			}
			template<typename U>
			unique_ptr& operator= (const unique_ptr<U> &pg){
				xPtr=pg.xPtr;
				pg.xPtr=nullptr;
				return *this;
			}
			~unique_ptr() {
				if(xPtr!=nullptr){
					delete xPtr;
				}
			}
			operator T*() const {
				assert(xPtr!=nullptr);
				return xPtr;
			}
			operator T&() const {
				assert(xPtr!=nullptr);
				return *xPtr;
			}
			T& operator *() const {
				assert(xPtr!=nullptr);
				return *xPtr;
			}
			T* get() const {return xPtr;}
            operator bool() const {return (xPtr==nullptr);}

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

/*	struct Vector
	{
		typedef Matrix<N, 1> type;
	};
*/
/*
	template<typename T>
	T& move (T& p){
		return p.move();
	}
*/
	template<typename T>
	T* move (unique_ptr<T>& p){
		return p.move();
	}
};
#endif //_ModernCPP_unique_ptr_H_

