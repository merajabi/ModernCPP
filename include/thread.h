#ifndef _ModernCPP_thread_H_
#define _ModernCPP_thread_H_
#include <iostream>
#include <pthread.h>
#include "type_traits.h"

namespace ModernCPP {
	struct ThreadDataBase {
		virtual ~ThreadDataBase(){};
	};

	template<typename FuncType,typename ParamType=void,typename dummy = ParamType>
	struct ThreadData : public ThreadDataBase {
		FuncType func;
		ParamType& param;
		ThreadData(FuncType func,ParamType& param):func(func),param(param){	std::cout << "T " << "param: " << param <<std::endl;};
		static void *function(void *data){
			std::cout << "function " << "T param: " << ((ThreadData<FuncType,ParamType>*)data)->param <<std::endl;
			((ThreadData<FuncType,ParamType>*)data)->func( ((ThreadData<FuncType,ParamType>*)data)->param );
		}
	};

	template<typename FuncType,typename ParamType>
	struct ThreadData <FuncType, ParamType, typename enable_if< is_same<ParamType, typename add_const<ParamType>::type >::value, ParamType >::type > : public ThreadDataBase {
		FuncType func;
		const ParamType& param;
		ThreadData(FuncType func, ParamType& param):func(func),param(param){std::cout << "const T " << "param: " << param <<std::endl;};
		static void *function(void *data){
			std::cout << "function " << "const T param: " << ((ThreadData<FuncType, typename ModernCPP::add_const<ParamType>::type >*)data)->param <<std::endl;
			((ThreadData<FuncType, typename ModernCPP::add_const<ParamType>::type >*)data)->func( ((ThreadData<FuncType, typename ModernCPP::add_const<ParamType>::type >*)data)->param );
		}
	};

	template<typename FuncType>
	struct ThreadData<FuncType,void,void> : public ThreadDataBase {
		FuncType func;
		ThreadData(FuncType func):func(func){std::cout << "FuncType " << std::endl;};
		static void *function(void *data){
			std::cout << "function " << std::endl;
			((ThreadData*)data)->func( );
		}
	};

	class thread {
		mutable bool status;
		mutable int error;
		mutable pthread_t threadId;
		mutable ThreadDataBase* data ;
		public:
/*
			template<typename FuncType,typename ParamType>
			thread(FuncType functor,ParamType param):status(false),data( new ThreadData<FuncType,ParamType>(functor,param) ){
				pthread_create( &threadId, NULL, &ThreadData<FuncType,ParamType>::function, data);
				status=true;
			}
*/

			template<typename FuncType,typename ParamType>
			thread(FuncType functor,ParamType& param):status(false),data( new ThreadData<FuncType,ParamType>(functor,param) ){
				std::cout << "thread " << threadId  << " T param: " << ((ThreadData<FuncType,ParamType>*)data)->param <<std::endl;
				pthread_create( &threadId, NULL, &ThreadData<FuncType,ParamType>::function, data);
				status=true;
			}

			template<typename FuncType,typename ParamType>
			thread(FuncType functor,const ParamType& param):status(false){
				data = new ThreadData<FuncType, typename ModernCPP::add_const<ParamType>::type >(functor,param) ;
				std::cout << "thread " << threadId << " const T param: " << ((ThreadData<FuncType, typename ModernCPP::add_const<ParamType>::type >*)data)->param <<std::endl;
				error=pthread_create( &threadId, NULL, &ThreadData<FuncType,typename ModernCPP::add_const<ParamType>::type>::function, data);
				if(error==0){;
					status=true;
				}else{
					std::cout << "error " << error <<std::endl;
				}
			}

			template<typename FuncType>
			thread(FuncType functor):status(false),data( new ThreadData<FuncType>(functor)){
				std::cout << "thread " << threadId <<std::endl;
				pthread_create( &threadId, NULL, &ThreadData<FuncType>::function, data);
				status=true;
			}
			thread(const thread& t):status(t.status),data(t.data),threadId(t.threadId){
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

