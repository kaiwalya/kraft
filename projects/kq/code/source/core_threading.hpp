namespace kq{
	namespace core{
		namespace threading{
			class Mutex: virtual public kq::core::debug::Log, private Resourcer{
				pthread_mutex_t mutex;

			public:
				enum Error{
					kErrNone,
					kErrSome,
				};

			protected:
				Error sleep(pthread_cond_t * cond){
					if(0 == pthread_cond_wait(cond, &mutex)){
						return kErrNone;
					}
					return kErrSome;
				}

				friend class Condition;

			public:

				Error initialize(){
					if(0 == pthread_mutex_init(&mutex, 0)){
						Resourcer::initialize();
						return kErrNone;
					}
					return kErrSome;
				}

				void finalize(){
					Resourcer::finalize();
					pthread_mutex_destroy(&mutex);
				}

				Error lock(){
					log("[%p]Lock %p...", pthread_self(), this);
					if(0 == pthread_mutex_lock(&mutex)){
						log("Locked\n");
						Resourcer::initialize();
						return kErrNone;
					}
					printf("Lock Error\n");
					return kErrSome;
				}

				Error unlock(){
					log("[%p]Lock %p...", pthread_self(), this);
					if(0 == pthread_mutex_unlock(&mutex)){
						log("UnLocked\n");
						Resourcer::finalize();
						return kErrNone;
					}
					log("UnLock Error\n");
					return kErrSome;
				}


			};

			class Condition: virtual public kq::core::debug::Log, private Resourcer{
				pthread_cond_t cond;
				bool bWake;

			public:
				enum Error{
					kErrNone,
					kErrSome,
				};
				Error initialize(){
					Error err = kErrSome;
					if(0 == pthread_cond_init(&cond, 0)){
						err = kErrNone;
						Resourcer::initialize();
						goto done;
					}
					done:
					bWake = false;
					return err;
				}

				void finalize(){
					Resourcer::finalize();
					pthread_cond_destroy(&cond);
				}

				Error sleep(Mutex * m){
					Error err = kErrNone;
					bWake = false;
					log("Cond %p and Mutex %p sleeping\n", this, m);
					while(!bWake && err == kErrNone){
						err = (m->sleep(&cond) == Mutex::kErrNone)?kErrNone:kErrSome;
						log("Cond %p and Mutex %p awake\n", this, m);
					}
					log("Cond %p and Mutex %p seperated\n", this, m);
					return err;
				}

				Error wake(){
					log("Cond %p waking\n", this);
					if(0 == pthread_cond_signal(&cond)){
						log("Cond %p woken\n", this);
						bWake = true;
						return kErrNone;
					}
					return kErrSome;
				}

				//TODO: Is wake all compatible with bWake?
				Error wakeAll(){
					if(0 == pthread_cond_broadcast(&cond)){
						bWake = true;
						return kErrNone;
					}
					return kErrSome;
				}
			};

			class ConditionMutex:public Condition,  public Mutex{


			public:
				Condition::Error initialize(){
					Condition::Error err = Condition::kErrSome;
					if(Condition::kErrNone == Condition::initialize()){
						if(Mutex::kErrNone == Mutex::initialize()){
							err = Condition::kErrNone;
							goto done;
						}
						Condition::finalize();
					}
					done:
					return err;
				}

				void finalize(){
					Mutex::finalize();
					Condition::finalize();
				}

				Condition::Error sleep(){
					return Condition::sleep(this);
				}
			};

			class ScopeMutex{
				Mutex * mutex;
			public:
				ScopeMutex(Mutex * mutex){this->mutex = mutex;mutex->lock();}
				~ScopeMutex(){mutex->unlock();}
			};

			class Thread{
			public:
				enum Error{
					kErrNone,
					kErrSome,
				};

				typedef pthread_t Handle;
				static Error create(Handle & t, void * (*fn)(void *), void * data){
					Error ret = kErrSome;
					if(0 == pthread_create(&t, 0, fn, data)){
						ret = kErrNone;
					}
					return ret;
				}

				static Error join(Handle &thread, void ** returnval){
					if(0 == pthread_join(thread, returnval)){
						return kErrNone;
					}
					return kErrSome;
				}

				static Error attach(Handle &thread){
					thread = pthread_self();
					return kErrNone;
				}
			};

		}
	}
}

