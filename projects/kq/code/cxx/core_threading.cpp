#include "core_threading.hpp"
#include "core_IntegerTypes.hpp"
#include "core_oops.hpp"
#include "pthread.h"
#include <memory>

using namespace kq::core::threading;


IThread::~IThread(){

}

IMutex::~IMutex(){

}

ICondition::~ICondition(){

}
/*
IContext::~IContext(){

}
*/

class Mutex: public IMutex{


public:
	pthread_mutex_t mutex;
	Mutex(){
		pthread_mutex_init(&mutex, 0);
	}
	Error lock(){
		pthread_mutex_lock(&mutex);
		kq::core::oops::Log l;
		l.log("mutex %p locked\n", this);
		return kErrNone;
	}

	Error unlock(){
		pthread_mutex_unlock(&mutex);
		kq::core::oops::Log l;
		l.log("mutex %p unlocked\n", this);
		return kErrNone;
	}

	~Mutex(){
		pthread_mutex_destroy(&mutex);
	}
};

class Condition: public ICondition{
	Mutex m;
	pthread_cond_t cond;
public:
	Condition(){
		pthread_cond_init(&cond, 0);
	}

	~Condition(){
		pthread_cond_destroy(&cond);
	}

	Error lock(){
		return m.lock();
	}
	Error unlock(){
		return m.unlock();
	}
	Error wait(){
		kq::core::oops::Log l;
		l.log("mutex %p unlocked and waiting on condition %p\n", &m, this);
		pthread_cond_wait(&cond, &m.mutex);
		l.log("mutex %p locked as condition %p was signaled\n", &m, this);
		return kErrNone;
	}
	Error signal(){
		kq::core::oops::Log l;
		l.log("signalling condition %p\n", this);
		pthread_cond_signal(&cond);
		return kErrNone;
	}
};

ScopeLock::ScopeLock(IMutex * m){
	this->m = m;
	m->lock();
}

ScopeLock::~ScopeLock(){
	m->unlock();
};

class Thread : public IThread/*, public kq::core::oops::Log*/{
	ThreadFunc func;
	void * data;

	enum State{
		kStateConstructed,
		kStateStarted,
		kStateReturned,
	};

	State s;
	pthread_t thread;
	Error returnErr;

	void work(){
		//log(">> Thread::work(this %p, func %p, data %p)\n", this, func, data);
		returnErr = func(data);
		//log("<< Thread::work(this %p, func %p, data %p)\n", this, func, data);
		s = kStateReturned;
	}
	static void * _work(void * data){
		((Thread *)data)->work();
		return nullptr;
	}
public:
	Thread(ThreadFunc f, void * d){
		func = f;
		data = d;
		memset(&thread, 0, sizeof(thread));
		s = kStateConstructed;
		//log("Thread::Thread(this %p, func %p, data %p)\n", this, func, data);
	}

	~Thread(){
		kill();
		//log("Thread::~Thread(this %p, func %p, data %p)\n", this, func, data);
	}

	Error start(){
		//log("Thread::start(this %p, func %p, data %p)\n", this, func, data);
		if(0 == pthread_create(&thread, 0, _work, this)){
			s = kStateStarted;
			return kErrNone;
		}
		return kErrOutOfMemory;
	}

	Error wait(Error * out){
		//log(">> Thread::wait(this %p, func %p, data %p)\n", this, func, data);
		Error ret;
		switch(s){
		case kStateConstructed:
			ret = kErrBadState;
			break;
		case kStateStarted:
			if(0 != pthread_join(thread, 0)){
				ret = kErrOutOfMemory;
				break;
			}
			//No break here!!

		case kStateReturned:
			(*out) = returnErr;
			ret = kErrNone;
		}
		//log("<< Thread::wait(this %p, func %p, data %p)\n", this, func, data);
		return ret;
	}

	Error kill(){
		//log("Thread::kill(this %p, func %p, data %p)\n", this, func, data);
		if(s == kStateStarted){
			if(0 == pthread_kill(thread, SIGKILL)){
				return kErrNone;
			}
			return kErrOutOfMemory;
		}
		return kErrBadState;
	}
};
/*
#include "ucontext.h"

class Context: public IContext{
	ThreadFunc f;
	void * d;
	ucontext_t c;

	static void * worker(void * data){
		Context * c = ((Context *)data);
		(*c->f)(d);

		//Should this function return ever???
		kq::core::oops::assume(false);
	}

public:

	Context(ThreadFunc func, void * data){
		getcontext(&c);
		c.uc_stack.ss_size = 16 * 1024;
		c.uc_stack.ss_sp = malloc(c.uc_stack.ss_size);
		if(c.uc_stack.ss_sp){
			c.uc_stack.ss_flags = 0;
			makecontext(c, worker, 1, data);
		}
	}

	Context(){

	}

	~Context(){

	}

	virtual Error load(IContext * saveOld){
		Error err = kErrNone;
		if(saveold){
			err = ((Context *)saveOld)->save();
		}

	}

	virtual Error save(){

	}
};
*/

Error IThread::constuctThread(IThread ** thread, ThreadFunc f, void * data){
	(*thread) = new Thread(f, data);
	return kErrNone;
}

Error ICondition::constructCondition(ICondition ** condition){
	*condition = new Condition();
	return kErrNone;
}

Error IMutex::constructMutex(IMutex ** mutex){
	*mutex = new Mutex();
	return kErrNone;
}

/*
Error IContext::constructCallContext(IContext ** context, void * (*func)(void *), void * data){
	*context = new Context(func, data);
	return kErrNone;
}

Error IContext::constructEmptyContext(IContext ** context, void * (*func)(void *), void * data){
	*context = new Context();
	return kErrNone;
}
*/


/*
namespace kq{
	namespace core{
		namespace threading{
			class Mutex: virtual public kq::core::oops::Log{
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

*/
