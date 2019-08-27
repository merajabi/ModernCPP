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
			void Lock(){
				if(pthread_mutex_lock( &m )==0){
					status = LOCKED;
				}
			}
			void Unlock(){
				if(pthread_mutex_unlock( &m )==0){
					status = UNLOCKED;
				}
			}
			LockStatus GetStatus() {return status;}
	};
	class lock_guard {
		mutex &m;
		public:
		lock_guard(mutex& m):m(m){
			m.Lock();
		}
		~lock_guard(){
			m.Unlock();
		}
	};


};




