namespace ModernCPP {
	class mutex {
		typedef enum {INIT,LOCKED,UNLOCKED} LockStatus;
		LockStatus status;
		pthread_mutex_t mutex;// = PTHREAD_MUTEX_INITIALIZER;
		public:
			mutex():status(INIT){
				if (pthread_mutex_init(&mutex, NULL) == 0) { 
					status = UNLOCKED;
				} 
			}
			~mutex(){
			    pthread_mutex_destroy(&mutex); 
			}
			void Lock(){
				if(pthread_mutex_lock( &mutex )==0){
					status = LOCKED;
				}
			}
			void Unlock(){
				if(pthread_mutex_unlock( &mutex )==0){
					status = UNLOCKED;
				}
			}
			LockStatus GetStatus() {return status;}
	};
	class lock_guard {
		mutex &m;
		public:
		lock_guard(Mutex& m):m(m){
			m.Lock();
		}
		~lock_guard(){
			m.Unlock();
		}
	};


};




