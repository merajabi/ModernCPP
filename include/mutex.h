#ifndef _ModernCPP_mutex_H_
#define _ModernCPP_mutex_H_

#include "typetraits.h"

namespace ModernCPP {
	class mutex {
		typedef enum {INIT,LOCKED,UNLOCKED} LockStatus;
		LockStatus status;
		pthread_mutex_t m;// = PTHREAD_MUTEX_INITIALIZER;
		public:
			mutex():status(INIT){
				if (pthread_mutex_init(&m, NULL) == 0) { 
					status = UNLOCKED;
				} 
			}
			~mutex(){
			    pthread_mutex_destroy(&m); 
			}
			void lock(){
				if(pthread_mutex_lock( &m )==0){
					status = LOCKED;
				}
			}
			void unlock(){
				if(pthread_mutex_unlock( &m )==0){
					status = UNLOCKED;
				}
			}
			LockStatus GetStatus() {return status;}
	};
	template<typename T>
	class lock_guard {
		T &m;
		lock_guard(const lock_guard &pg);
		lock_guard& operator= (const lock_guard &pg);
		lock_guard(lock_guard &pg);
		lock_guard& operator= (lock_guard &pg);
		public:
		lock_guard(T& m):m(m){
			m.lock();
		}
		~lock_guard(){
			m.unlock();
		}
		T& operator ->() {
			return m;
		}
		T& operator ->() const {
			return m;
		}
	};
}

#endif //_ModernCPP_mutex_H_


