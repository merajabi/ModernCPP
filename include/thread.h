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
		}
	};

	template<typename FuncType>
	struct ThreadData<FuncType,void> : public ThreadDataBase {
		FuncType func;
		ThreadData(FuncType func):func(func){};
		static void *function(void *data){
			((ThreadData*)data)->func( );
		}
	};

	class thread {
		bool status;
		pthread_t threadId;
		ThreadDataBase* data ;
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

			~thread(){
				if(status){
					Join();
					//Detach();
				}
				delete data;
			}
			void Join(){
				if(status){
					pthread_join( threadId, NULL);
					status=false;
				}
			}
			void Detach() {
				if(status){
					pthread_detach(threadId);
					status=false;
				}
			}					
	};

};
