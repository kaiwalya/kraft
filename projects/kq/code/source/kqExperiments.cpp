#include "core.hpp"

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

static void remoteCall(void * stacklocation, size_t stacksize, void (*fn)(void *), void * data) __attribute__((noinline));
static void remoteCall(void * stacklocation, size_t stacksize, void (*fn)(void *), void * data){

	//Should be optimized at compile time in release
	if(sizeof(void *) == sizeof(kq::core::ui64)){

		//Prepare stack
		{
			volatile register kq::core::ui64 sp asm("rsp");
			sp = (kq::core::ui64)stacklocation + stacksize;
			//Dont know why the compiler optimizes the following line out?
			//sp = sp & 0xFFFFFFFFFFFFFFF0;
		}

		fn(data);

		//Undo stack effects
		{
			//Without this last line leave instruction calls are wrong
			//Also makes sure that in release builds, fn(data) is not a tail call
			//If this is not present leave instruction will execute before the fn call and restore the stack
			volatile register kq::core::ui64 bp asm("rbp");
			volatile register kq::core::ui64 sp asm("rsp");
			sp = bp;
		}
	}

}

namespace kq{
	namespace core{
		namespace data{
			class IDMap{
				memory::MemoryWorker & mem;
				BPlusTree tree;
				typedef ui32 IDVal;
				IDVal m_iNext;
				IDVal m_nAlive;
			public:
				IDMap(memory::MemoryWorker & memworker)
				:mem(memworker), tree(memworker, sizeof(IDVal)), m_iNext(1), m_nAlive(0){}

				~IDMap(){}

				IDVal getAliveIDCount(){return m_nAlive;}

				template<typename t> IDVal create(t * p){
					IDVal ret = 0;
					if(tree.map(&(m_iNext), p)){
						ret = m_iNext;
						m_nAlive++;
						m_iNext++;
					}
					return ret;
				}

				template<typename t> t * get(const IDVal id){
					t * ret = 0;
					if(id && id < m_iNext){
						ret = tree.lookup(&id);
					}
					return ret;
				}

				template<typename t> t * destroy(const IDVal id){
					t * ret = 0;
					if(id && id < m_iNext){
						if(tree.map(&id, 0, (void **)&ret)){
							m_nAlive--;
						}
					}
					return ret;
				}

			public:
				kq::core::data::BPlusTree::Path getIterator(){
					return kq::core::data::BPlusTree::Path(&tree);
				}

			};

		}
	}
}

#include "pthread.h"

#include "stdarg.h"
#include "assert.h"

namespace kq{
	namespace core{
		namespace debug{
			void assume(kq::core::ui32 iVal, ...){
				assert(iVal);
			}

			class Log{
				bool bLogging;
			public:
				Log(){bLogging = false;}
				virtual void enableLogging(){bLogging = true;}
				virtual void disableLogging(){bLogging = false;}
				void log(const char * format, ...){
					va_list args;
					va_start (args, format);
					if(bLogging)
						vprintf (format, args);
					va_end (args);
				}
			};
		}

		namespace system{

			class Resourcer{

			public:
				typedef kq::core::ui32 ResourceCount;

				class ResourceManager:virtual kq::core::debug::Log{
					volatile static kq::core::ui32 nResourcers;
				public:
					ResourceManager(){enableLogging();}
					~ResourceManager(){
						log("%d resourcers leaked\n", nResourcers);
					}
					void operator+=(ResourceCount c){
						nResourcers+=c;
					}
					void operator-=(ResourceCount c){
						nResourcers-=c;
					}
				};

				static ResourceManager manager;
				kq::core::ui32 nResources;
			public:
				Resourcer():nResources(0){}
				~Resourcer(){kq::core::debug::assume(!nResources);}
				void initialize(int iCount = 1){manager+=iCount;nResources+=iCount;}
				void finalize(int iCount = 1){manager-=iCount;nResources-=iCount;}
			};

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
		namespace flows{

			enum Error{
				kErrNone,
				kErrPowerOnOff,
				kErrBusy,
				kErrDeadEnd,
				kErrUnexpectedState,
				kErrOutOfMemory,
			};


			class IData{
			protected:
				virtual IData * clone();
				virtual void finalize();
			};

			typedef kq::core::ui32 Port;
			typedef kq::core::ui32 DataCount;
			enum Direction{
				kDirectionIn,
				kDirectionOut,
			};

			struct Message{
				Port port;
				Direction direction;
				IData ** arrData;
				DataCount min;
				DataCount max;
				void set(Port port, Direction d, IData ** arrData, DataCount min, DataCount max);
			};

			class IPorts{
			public:
				virtual Error prefetch(Port iPort, DataCount nData) = 0;
				virtual Error performIO(Message *) = 0;
			};

			typedef Error (* Processor)(const IPorts *, const Message *);

			class ISocket: virtual public kq::core::debug::Log{

			public:
				enum Operation{
					kSocketNone,
					kSocketCreate,
					kSocketInsertProcessor,
					kSocketEjectProcessor,
					kSocketConnect,
					kSocketPrefetch,
					kSocketPerformIO,
					kSocketDestroy,
				};

				struct OperationData{};
				struct OperationCreate:public OperationData{kq::core::memory::MemoryWorker * mem;};
				struct OperationInserProcessor:public OperationData{};

			};

			class DHSSocket:public ISocket{
				kq::core::memory::MemoryWorker & mem;

				kq::core::system::ConditionMutex mx;


				kq::core::system::Condition cCitizen;
				kq::core::system::Thread::Handle hCitizen;
				bool bShuttingDown;
				bool bNewData;
				Operation operation;
				OperationData * operationData;
				Error operationError;

				virtual Error dooperation_citizen(Operation operation, OperationData * data){
					Error err = kErrOutOfMemory;
					log("Operation Done\n");
					return err;
				}


				void citizenwork(){
					{
						kq::core::system::ScopeMutex _0(&this->mx);
						while(!bShuttingDown){
							while(!bNewData && !bShuttingDown)
								cCitizen.sleep(&this->mx);
							if(bShuttingDown){
								break;
							}
							operationError = dooperation_citizen(operation, operationData);
							bNewData = false;
							mx.wake();
						}
					}
					cCitizen.finalize();
				}

				static void * _citizen_work(void * obj){
					((DHSSocket *)obj)->citizenwork();
					return 0;
				}

			public:
				DHSSocket(kq::core::memory::MemoryWorker & memworker):mem(memworker), hCitizen(0){
					mx.initialize();
					mx.enableLogging();
					enableLogging();

				}

				~DHSSocket(){
					bShuttingDown = true;
					{
						kq::core::system::ScopeMutex _0(&this->mx);
						cCitizen.wake();
					}
					if(hCitizen)kq::core::system::Thread::join(hCitizen, 0);
					mx.finalize();
				}

				virtual Error dooperation(Operation operation, OperationData * data){
					Error err;

					if(!hCitizen){
						kq::core::system::ScopeMutex _0(&this->mx);

						if(!hCitizen){
							bShuttingDown = false;
							bNewData = false;
							cCitizen.initialize();
							cCitizen.enableLogging();
							if(kq::core::system::Thread::kErrNone != kq::core::system::Thread::create(hCitizen, &_citizen_work, (void *)this)){
								//Couldnot start citizen
								err = kErrDeadEnd;
								goto done;
							}

						}

					}
					{
						kq::core::system::ScopeMutex _0(&this->mx);
						this->operation = operation;
						this->operationData = data;
						bNewData = true;
						cCitizen.wake();
						while(bNewData){
							mx.sleep();
						}
						err = operationError;


					}
					done:
					log("doOperation Err = %u\n", (unsigned int)err);
					return err;
				}
			};

			class SocketData: public IPorts{
				kq::core::memory::MemoryWorker & mem;

				enum ProcessorState{
					kStateOff,
					kStateOn,
					kStateBusy,
				};
				ProcessorState state;

				Processor processor;
				void * stackstart;
				size_t stacksize;
				kq::core::system::Mutex mxExternalEntry;

				class IPortsProcessor: public IPorts{
					SocketData * socketdata;
					friend class SocketData;
				public:
					virtual Error prefetch(Port iPort, DataCount nData);
					virtual Error performIO(Message *);

				};

				IPortsProcessor processorports;

				Error processOuter(const Message *);
				Error processInner(const Message *);

				kq::core::system::Mutex mxConnections;
				kq::core::data::BPlusTree connections;

			public:
				SocketData(kq::core::memory::MemoryWorker &);
				~SocketData();
				Error ejectProcessor();
				Error insertProcessor(Processor p);
				virtual Error prefetch(Port iPort, DataCount nData);
				virtual Error performIO(Message *);
				Error connect(Port iPort, SocketData * other, Port iOther);

			};
		}
	}
}

volatile kq::core::ui32 kq::core::system::Resourcer::ResourceManager::nResourcers = 0;
kq::core::system::Resourcer::ResourceManager kq::core::system::Resourcer::manager;

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;
using namespace kq::core::data;
using namespace kq::core::flows;

IData * IData::clone() {return 0;}
void IData::finalize(){}

void Message::set(Port port, Direction d, IData ** arrData, DataCount min, DataCount max)
{
	this->port = port;
	this->arrData = arrData;
	this->min = min;
	this->max = max;
	this->direction = d;
}

SocketData::SocketData(kq::core::memory::MemoryWorker & memworker):
		mem(memworker),
		state(kStateOff),
		processor(0),
		connections(memworker, sizeof(Port))
{
	mxExternalEntry.initialize();
	mxConnections.initialize();
	processorports.socketdata = this;
}

SocketData::~SocketData(){
	ejectProcessor();
}

struct ProcessParcel{
	SocketData * s;
	Error (SocketData::*f)(const Message *);
	Error e;
	const Message * m;
};

Error SocketData::processInner(const Message * m){
	return (*processor)(&processorports, m);
}

void processparceltransfer(ProcessParcel * p){
	//ProcessParcel * p = (ProcessParcel *) v;
	p->e = (p->s->*p->f)(p->m);
}

Error SocketData::processOuter(const Message * m){
	ProcessParcel p;
	p.s = this;
	p.f = &SocketData::processInner;
	p.e = kErrNone;
	p.m = m;

	remoteCall(stackstart, stacksize, (void (*)(void *))processparceltransfer, &p);
	return p.e;
}

Error SocketData::ejectProcessor(){
	Error err;
	mxExternalEntry.lock();
	if(state != kStateOff){
		Message m;
		m.set(0, kDirectionIn, 0, 0, 0);
		if(processOuter(&m) == kErrNone){
			state = kStateOff;
			mem(stackstart, 0);
			err = kErrNone;
		}
		else{
			err = kErrPowerOnOff;
		}
	}
	if(state == kStateOff){
		processor = 0;
		err = kErrNone;
	}
	mxExternalEntry.unlock();

	return err;
}

Error SocketData::insertProcessor(Processor p){
	Error err;
	if(kErrNone == ejectProcessor()){
		processor = p;
	}
	return err;
}

Error SocketData::prefetch(Port iPort, DataCount nData){
	return kErrNone;
}

Error SocketData::performIO(Message * m){
	Port & i = m->port;

	Error err = kErrNone;
	//Port 0 is not open to public
	if(i == 0){
		err = kErrDeadEnd;
	}
	else{
		kq::core::system::ScopeMutex enter(&mxExternalEntry);
		//Check if the processor has booted
		if(state == kStateOff){
			stacksize = 4096;
			stackstart = mem(0, stacksize);
			if(stackstart){
				Message m;
				m.set(0, kDirectionIn, 0, 0, 1);
				err = processOuter(&m);
				if(err == kErrNone){
					state = kStateOn;
				}
				else{
					err = kErrPowerOnOff;
				}
			}
			else{
				err = kErrOutOfMemory;
			}
		}

		if(state == kStateOn){
			state = kStateBusy;
			err = processOuter(m);
			state = kStateOn;
		}
	}

	return err;
}

Error SocketData::IPortsProcessor::prefetch(Port iPort, DataCount nData){
	return kErrNone;
}
Error SocketData::IPortsProcessor::performIO(Message * m){
	return kErrUnexpectedState;
}

Error SocketData::connect(Port iPort, SocketData * other, Port iOther){
	if(iPort == 0 || iOther == 0){
		return kErrDeadEnd;
	}

	//this should be > other, to avoid dead locks
	if(this < other){
		return other->connect(iOther, this, iPort);
	}

	Error err;
	//first make a broad division, make or break?
	if(other){
		//connect/reconnect
		kq::core::data::BPlusTree::Path p(&connections);

	}
	else{
		//disconnect

	}

	return err;

}



/*
class TestProcessor:public Processor{

public:
	TestProcessor(){

	}

	~TestProcessor(){

	}

	struct Return:public IData{
		int ret;
	};

	int entry(){

		Return r;
		IData * pr = &r;
		Flow f;
		f.set(1, 1, 1, &pr);
		if(doFlow(&f) == kErrNone){
			return r.ret;
		}
		return -1;
	}
protected:
	virtual Error handleFlow(const Flow * flow){
		printf("FlowIndex %u Data %p\n", (unsigned int)flow->fid, flow->data);
		return kErrNone;
	}


};

*/

Error consumer(const IPorts * ports, const Message * msg){
	printf("Consumer %p %p[%u %p %u %u]\n", ports, msg, (ui32)msg->port, msg->arrData, (ui32)msg->min, (ui32)msg->max);
	Error err;
	switch(msg->port){
	case 0:
		err = kErrNone;
		break;
	case 1:
	default:
		err = kErrDeadEnd;
	}
	return err;
}

Error producer(const IPorts * ports, const Message * msg){
	printf("Producer %p %p[%u %p %u %u]\n", ports, msg, (ui32)msg->port, msg->arrData, (ui32)msg->min, (ui32)msg->max);
	return kErrNone;

}

int main(int /*argc*/, char ** /*argv*/){
	//LOGINOUT;
	int ret;

	//Create std allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	{
		MemoryWorker mem = memStd;
		//PooledMemoryAllocator allocPool(memStd);
		//allocPool.getMemoryWorker(mem);
		{


			/*
			TestProcessor p;
			ret = p.entry();
			*/

			/*
			flows::SocketData p(mem);
			flows::SocketData c(mem);
			p.insertProcessor(producer);
			c.insertProcessor(consumer);
			p.connect(1, &c, 1);
			Message m;
			m.set(1, kDirectionOut, 0, 0, 0);
			c.performIO(&m);
			*/

			flows::DHSSocket dhs(mem);
			flows::Error err = kErrNone;
			{
				DHSSocket::OperationCreate o;
				o.mem = &mem;
				err = dhs.dooperation(DHSSocket::kSocketCreate, &o);
			}

		}
	}



	return ret;
}



