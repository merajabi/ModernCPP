#ifndef _ModernCPP_thread_H_
#define _ModernCPP_thread_H_

#include "typetraits.h"
#include "smartguard.h"

#include <iostream>
#include <exception>
#include <pthread.h>
		
namespace ModernCPP {
	struct system_error : public std::runtime_error {
		system_error(const std::string& str):std::runtime_error(str){}
	};

	struct ThreadDataBase {
		virtual ~ThreadDataBase(){};
	};

	template<typename FuncType,typename ParamType=void>
	struct ThreadData : public ThreadDataBase {
		FuncType func;
		ParamType param;

		ThreadData(FuncType func,ParamType param):func(func),param(param){};

		static void* function(void *data){
			((ThreadData*)data)->func( ((ThreadData*)data)->param );
			return 0;
		}
	};

	template<typename FuncType>
	struct ThreadData<FuncType,void> : public ThreadDataBase {
		FuncType func;

		ThreadData(FuncType func):func(func){};

		static void* function(void *data){
			((ThreadData*)data)->func( );
			return 0;
		}
	};

	class thread {
		mutable pthread_t threadId;
		mutable SmartGuard<ThreadDataBase> data ;
		mutable bool status;

		public:

			template<typename FuncType,typename ParamType>
			thread(FuncType functor,ParamType param):threadId(0),data( new ThreadData<FuncType,ParamType>(functor,param) ),status(false){
				int error = pthread_create( &threadId, NULL, &ThreadData<FuncType,ParamType>::function, *data);

				if(error!=0){
					throw system_error("thread");
				}
				status=true;
			}

			template<typename FuncType>
			thread(FuncType functor):threadId(0),data( new ThreadData<FuncType>(functor)),status(false){
				int error = pthread_create( &threadId, NULL, &ThreadData<FuncType>::function, *data);

				if(error!=0){
					throw system_error("thread");
				}
				status=true;
			}

			thread(const thread& t):threadId(t.threadId),data(t.data),status(t.status) { // its move constructor
				t.status=false;
				t.threadId=-1;
				t.data=nullptr;
			}

			const thread& operator = (const thread& t) const {  // its move assignment operator
				status=t.status;
				data=t.data;
				threadId=t.threadId;
				t.status=false;
				t.threadId=-1;
				t.data=nullptr;
				return *this;
			}

			~thread(){
				if(status){
					pthread_cancel(threadId);
					status=false;
					std::terminate();
				}
			}

			void join() const {
				if(!status){
					throw system_error("joinable");
				}
				pthread_join( threadId, NULL);
				status=false;
			}

			void detach() const {
				if(!status){
					throw system_error("joinable");
				}
				pthread_detach(threadId);
				status=false;
			}

			bool joinable() const {
				return status;
			}					
	};

}
#endif //_ModernCPP_thread_H_

