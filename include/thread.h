#ifndef _ModernCPP_thread_H_
#define _ModernCPP_thread_H_

#include <pthread.h>
#include "type_traits.h"

namespace ModernCPP {
	struct ThreadDataBase {
		virtual ~ThreadDataBase(){};
	};

	template<typename FuncType,typename ParamType=void>
	struct ThreadData : public ThreadDataBase {
		FuncType func;
		ParamType param;
		ThreadData(FuncType func,ParamType param):func(func),param(param){};
		static void *function(void *data){
			((ThreadData*)data)->func( ((ThreadData*)data)->param );
			return 0;
		}
	};

	template<typename FuncType>
	struct ThreadData<FuncType,void> : public ThreadDataBase {
		FuncType func;
		ThreadData(FuncType func):func(func){};
		static void *function(void *data){
			((ThreadData*)data)->func( );
			return 0;
		}
	};

	class thread {
		mutable bool status;
		mutable pthread_t threadId;
		mutable ThreadDataBase* data ;
		public:

			template<typename FuncType,typename ParamType>
			thread(FuncType functor,ParamType param):status(false),data( new ThreadData<FuncType,ParamType>(functor,param) ){
				pthread_create( &threadId, NULL, &ThreadData<FuncType,ParamType>::function, data);
				status=true;
			}
			template<typename FuncType>
			thread(FuncType functor):status(false),data( new ThreadData<FuncType>(functor)){
				pthread_create( &threadId, NULL, &ThreadData<FuncType>::function, data);
				status=true;
			}
			thread(const thread& t):status(t.status),threadId(t.threadId),data(t.data){
				t.status=false;
				t.threadId=-1;
				t.data=nullptr;
			}
			thread& operator = (const thread& t){
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
				}
				delete data;
			}
			void join(){
				if(status){
					pthread_join( threadId, NULL);
					status=false;
				}
			}
			void detach() {
				if(status){
					pthread_detach(threadId);
					status=false;
				}
			}
			bool joinable(){
				return status;
			}					
	};

};
#endif //_ModernCPP_thread_H_

