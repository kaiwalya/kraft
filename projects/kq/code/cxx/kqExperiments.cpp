#include "core.hpp"

/*
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
*/

/*
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

*/


/*
namespace kq{
	namespace core{

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

*/


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
/*
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
*/
#include <assert.h>
#include <memory>
#define nullptr (0);
namespace kq{
	namespace flows3{
		//Class RangeTree, should move to data namespace
		class RangeTree{
			struct Node{
				void * from;
				void * to;
				void * data;
				Node * next;
			};
			
			Node * first;
			
			Node * find(void * mem){
				Node * n = first;
				for(;n;){
					if(n->from <= mem && mem <= n->to){
						return n;
					}
					n = n->next;
					
					if(n == first){
						break;
					}
				}
				return nullptr;
			}
		public:
			enum Error{
				kErrNone,
				kErrOverlappingRange,
				kErrUnknownRange,
			};
			

			RangeTree(){
				first = nullptr;
			}
			
			Error addRange(void * from, void * to, void * data){
				if(!find(from)){
					if(!find(to)){
						Node * newnode = new Node();
						newnode->from = from;
						newnode->to = to;
						newnode->data = data;
						if(!first){
							first = newnode;
						}
						newnode->next = first;
						first = newnode;
						return kErrNone;
					}
				}
				return kErrOverlappingRange;
			}
			
			Error removeRange(void * from, void * to, void ** data){
				Node * n1 = find(from);
				Node * n2 = find(to);
				if(n1 == n2){
					if(n1){
						if(data){
							*data = n1->data;
						}
						Node * next = n1->next;
						(*n1) = (*next);
						delete next;
						if(next == first){
							assert(n1 == first);
							first = nullptr;
						}
						return kErrNone;
					}
					return kErrUnknownRange;
				}
				return kErrOverlappingRange;
			}
			
			Error findRange(void * val, void ** data){
				Node * n = find(val);
				if(n){
					if(data){
						*data = n->data;
					}
					return kErrNone;
				}
				return kErrUnknownRange;
			}
		};
		//Class RangeTree ends;
		
		
		//Public API begin
		typedef int ProcessorID;
		typedef int PortID;
		typedef int LinkID;

		
		enum Error{
			kErrNone,
			kErrOutOfMemory,
			kErrUndefinedID,
		};
		typedef Error (*ProcessorFunc)();
		
		struct FlowsAPI{

			Error (*processor_create)(ProcessorID *, ProcessorFunc);
			Error (*processor_wait)(ProcessorID id);
			Error (*processor_link)(LinkID *, ProcessorID, PortID, ProcessorID, PortID);
			Error (*processor_unlink)(LinkID);
		};
		
		
		Error flows_init(FlowsAPI *);
		Error flows_fini();
		
		//Public API End
		
	
		//Processor Class

		class Global;
		class Processor{
			friend class Global;
			
			typedef unsigned int Count;
			
			static Global * g;
			
			//Look at the stack to get the current processor
			static Error findCurrent(Processor **);
		public:
			static Error loadClass(FlowsAPI * api);
			static Error unloadClass();
			static Error processor_create(ProcessorID *, ProcessorFunc);
			static Error processor_wait(ProcessorID id);
			static Error processor_link(LinkID *, ProcessorID, PortID, ProcessorID, PortID);
			static Error processor_unlink(LinkID);
			
		private:
			ProcessorID pid;
			ProcessorFunc func;
			Processor * parent;
			Count childCount;
			Processor * children;
			
			
			Processor();
			Processor(Processor * owner, ProcessorFunc func);
			
			//Convert processor to ID
			ProcessorID toID();
			//Find a child with a given ID
			Error findChildWithID(Processor ** child, ProcessorID);
			//Find a link with a givenID
			Error findLinkWithID(LinkID);
			
			Error create(Processor ** out, ProcessorFunc);
			Error wait();
			Error link(LinkID *, Processor * child1, PortID port1, Processor * child2, PortID port2);
			Error unlink(LinkID);
			

			
		};
		
		struct Global{
			RangeTree rangeTree;
			Processor rootProcessor;
			Global();
			~Global();
		};
		
		Global * Processor::g = nullptr;
		
		Error Processor::create(Processor **out, ProcessorFunc func){
			Processor * pid = new Processor(this, func);
			*out = pid;
			return kErrNone;
		}
		
		Error Processor::wait(){
			return kErrOutOfMemory;
		}
		
		Error Processor::link(LinkID * link, Processor * child1, PortID portid1, Processor * child2, PortID portid2){
			return kErrOutOfMemory;
		}
		
		Error Processor::unlink(LinkID){
			return kErrOutOfMemory;
		}
		
		Error Processor::processor_create(ProcessorID * id, ProcessorFunc func){
			Error err;
			Processor * pthis;
			if(kErrNone == (err = findCurrent(&pthis))){
				Processor * child;
				if(kErrNone == (err = pthis->create(&child, func))){
					(*id) = child->toID();
				}
			}
			return err;
		}
		
		Error Processor::processor_wait(ProcessorID id){
			Error err;
			Processor * pthis;
			if(kErrNone == (err = findCurrent(&pthis))){		
				Processor * child;
				if(kErrNone == (err = pthis->findChildWithID(&child, id))){
					err = child->wait();
				}
			}
			return err;
		}
		
		Error Processor::processor_link(LinkID * lid, ProcessorID pid1, PortID portid1, ProcessorID pid2, PortID portid2){
			Error err;
			Processor * pthis;
			if(kErrNone == (err = findCurrent(&pthis))){					
				Processor * child1;
				if(kErrNone == (err = pthis->findChildWithID(&child1, pid1))){
					Processor * child2;
					if(kErrNone == (err = pthis->findChildWithID(&child2, pid2))){
						err = pthis->link(lid, child1, portid1, child2, portid2);
					}
				}
			}
			return err;
		}
		
		Error Processor::processor_unlink(LinkID lid){
			Error err;
			Processor * pthis;
			if(kErrNone == (err = findCurrent(&pthis))){
				err = pthis->unlink(lid);
			}
			return err;
		}

		
		Processor::Processor(){
			//This constructor should be used only on the root processor
			assert(this == &g->rootProcessor);
			parent = nullptr;
			childCount = 0;
		}
		
		Processor::Processor(Processor * owner, ProcessorFunc func){
			parent = owner;
			childCount = 0;
		}
		
		Global::Global(){
			
		}
		
		Global::~Global(){
			
		}
		
		Error Processor::loadClass(FlowsAPI * api){
			if(!g){
				g = (Global *)malloc(sizeof(Global));
				if(g){
					new (g)Global();
					api->processor_create = Processor::processor_create;
					api->processor_wait = Processor::processor_wait;
					api->processor_link = Processor::processor_link;
					api->processor_unlink = Processor::processor_unlink;
					return kErrNone;
				}
				return kErrOutOfMemory;
			}
			return kErrNone;
		}
		
		Error Processor::unloadClass(){
			if(g){
				g->~Global();
				free(g);
				g = nullptr;
			}
			return kErrNone;
		}
		
		Error Processor::findCurrent(Processor ** processor){
			RangeTree::Error rangeerr = g->rangeTree.findRange(&processor, (void **)processor);
			
			if(rangeerr == RangeTree::kErrNone){
				return kErrNone;
			}
			assert(g != 0);
			*processor = &g->rootProcessor;
			return kErrNone;
		}
		
		Error Processor::findChildWithID(Processor ** child, ProcessorID id){
			Count c = childCount;
			Count i = 0;
			while(i < c){
				if(children[i].pid == id){
					*child = &children[i];
				}
				i++;
			}
			return kErrUndefinedID;
		}
		
		ProcessorID Processor::toID(){
			return pid;
		}
		
		//Processor class end
	}
}

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;

using namespace kq::core::flow;

class FlowSessionClient:public IFlowSessionClient{
public:

};


flows3::Error test1(){
	return flows3::kErrNone;
}

flows3::Error test2(){
	return flows3::kErrNone;
}

int main(int /*argc*/, char ** /*argv*/){
	//LOGINOUT;
	int ret;

	//Create std allocator
	kq::core::memory::StandardLibraryMemoryAllocator allocStd;
	kq::core::memory::MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	{
		kq::core::memory::MemoryWorker mem = memStd;
		//kq::core::memory::PooledMemoryAllocator allocPool(memStd);
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

			/*
			flows::DHSSocket dhs(mem);
			flows::Error err = kErrNone;
			{
				DHSSocket::OperationCreate o;
				o.mem = &mem;
				err = dhs.dooperation(DHSSocket::kSocketCreate, &o);
			}
			*/
			{
				flows3::FlowsAPI api;
				flows3::Processor::loadClass(&api);
				flows3::ProcessorID pid[2];
				
				api.processor_create(&pid[0], test1);
				api.processor_create(&pid[1], test2);
				
				api.processor_link(0, pid[0], 1, pid[1], 1);
				
				api.processor_wait(pid[1]);
				
				flows3::Processor::unloadClass();
			}

			{
				Pointer<IFlowSessionServer> pServer;
				{
					Pointer<IFlowSession> pSession;
					IFlowSession::FlowsSessionInitOptions o;
					o.mem = &mem;
					o.pClient = kq_core_memory_workerRefCountedClassNew(mem, FlowSessionClient);

					if(IFlowSession::createFlowSession(&o, pSession) == kErrNone){
						pServer = pSession->getServer();
					}
				}
				const unsigned char sProcessor_RandomNumberGenerator[] = "Random_Number_Generator";
				const unsigned char sProcessor_RandomNumberSink[] = "Random_Number_Sink";
				ProcessorType type;
				type.location = sProcessor_RandomNumberGenerator;
				type.length = sizeof(sProcessor_RandomNumberGenerator);
				if(pServer){
					Pointer<ISocket> pSource;
					if(kErrNone == pServer->createSocket(&type, pSource)){
						Pointer<ISocket> pDest;
						type.location = sProcessor_RandomNumberSink;
						type.length = sizeof(sProcessor_RandomNumberSink);
						if(kErrNone == pServer->createSocket(&type, pDest)){
							Pointer<IConnection> pConnection;
							if(kErrNone == pSource->createConnection(1, pDest, 1)){
								pConnection->recommendBufferLength(100);
							}
						}
					}
				}
			}
			/*
			IFlowServer * pFlowServer;
			FlowClient flowClient;
			f.createFlowSession(&flowClient, &pFlowServer);
			*/



		}
	}

	return ret;
}



