#include "core_flow.hpp"
#include "memory"
#include "malloc.h"
#include "core_memory.hpp"
#include "core_oops.hpp"
#include "core_threading.hpp"

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;

#include <assert.h>
#include <memory>

namespace kq{
	namespace core{

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



			//Processor Class
			struct Global;
			class Processor{
				friend struct Global;
				typedef unsigned int Count;
				static Global * g;

			public:
				static Error loadClass(FlowsAPI * api);
				static Error unloadClass();
				static Error processor_create(ProcessorID *, ProcessorFunc);
				static Error processor_wait(ProcessorID id);
				static Error processor_link(LinkID *, ProcessorID, PortID, ProcessorID, PortID);
				static Error processor_unlink(LinkID);

			private:
				//Look at the stack to get the current processor
				static Error findCurrent(Processor **);
				static kq::core::threading::Error _workerfunc(void *);

				struct Link{
					Processor * processors[2];
					PortID port[2];
					Link * next;
				};

				struct Resource_Thread{
					kq::core::threading::IThread * thread;

					bool bControllerProceed;
					kq::core::threading::ICondition * conditionControllerWaits;

					bool bThreadProceed;
					kq::core::threading::ICondition * conditionThreadWaits;

					Count threadID;
					Processor * processor;
				};
				struct Resources{
					Processor::Count nthreads;
					Resource_Thread * threads;
				};


				ProcessorID pid;
				ProcessorID lastchildpid;
				ProcessorFunc func;
				Processor * parent;
				Count childCount;
				Processor * children;
				Link * firstLink;
				Resources * resources;


				Processor();
				Processor(Processor * owner, ProcessorID id, ProcessorFunc func);

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

				kq::core::threading::Error workerfunc(Resource_Thread *);

			};

			struct Global{
				RangeTree rangeTree;
				Processor rootProcessor;

				Global();
				~Global();
			};

			Global * Processor::g = nullptr;

			Error Processor::create(Processor **out, ProcessorFunc func){
				Processor * pid = new Processor(this, ++lastchildpid, func);
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


			kq::core::threading::Error Processor::workerfunc(Resource_Thread * rt){
				kq::core::oops::Log l;
				l.log("Thread %d booting\n", (int)rt->threadID);

				kq::core::threading::ScopeLock slThread(rt->conditionThreadWaits);
				//Unblock the controller, it should always be waiting for the thread to start
				{
					kq::core::threading::ScopeLock slController(rt->conditionControllerWaits);
					kq::core::oops::assume(rt->bControllerProceed == false);
					rt->bControllerProceed = true;
					rt->conditionControllerWaits->signal();
					l.log("Thread %d unblocked processor\n", (int)rt->threadID);
				}

				l.log("Thread %d waiting\n", (int)rt->threadID);
				while(!rt->bThreadProceed){
					rt->conditionThreadWaits->wait();
				}
				l.log("Thread %d done waiting\n", (int)rt->threadID);

				return kq::core::threading::kErrNone;
			}


			kq::core::threading::Error Processor::_workerfunc(void * data){
				Resource_Thread * rt = (Resource_Thread *)data;
				return rt->processor->workerfunc(rt);
			}

			Processor::Processor(){
				//This constructor should be used only on the root processor
				kq::core::oops::assume(this == &g->rootProcessor);
				pid = 0;
				lastchildpid = 0;
				this->func = func;
				parent = nullptr;
				childCount = 0;
				firstLink = 0;

				//Create top level resources
				{
					//Threads
					{
						const Count maxThreads = 4;
						resources = (Resources *)malloc(sizeof(Resources));
						if(resources){
							new (resources) Resources;
							resources->threads = (Resource_Thread *)malloc(sizeof(Resource_Thread) * maxThreads);
							if(resources->threads){
								resources->nthreads = 0;
								while(resources->nthreads < maxThreads){
									Resource_Thread & rt = resources->threads[resources->nthreads];
									rt.threadID = resources->nthreads;
									rt.processor = this;
									kq::core::threading::Error err;
									err = kq::core::threading::ICondition::constructCondition(&rt.conditionThreadWaits);
									if(kq::core::threading::kErrNone == err){
										err = kq::core::threading::ICondition::constructCondition(&rt.conditionControllerWaits);
										if(kq::core::threading::kErrNone == err){
											err = kq::core::threading::IThread::constuctThread(&rt.thread, _workerfunc, &rt);
											if(kq::core::threading::kErrNone == err){
												rt.bThreadProceed = false;
												rt.bControllerProceed = false;
												kq::core::threading::ScopeLock slController(rt.conditionControllerWaits);
												err = rt.thread->start();
												if(kq::core::threading::kErrNone == err){
													resources->nthreads++;
													while(!rt.bControllerProceed){
														rt.conditionControllerWaits->wait();
													}
													continue;
												}
												delete rt.thread;
											}
											delete rt.conditionControllerWaits;
										}
										delete rt.conditionThreadWaits;
									}
									break;
								}
							}
						}
					}
				}

			}

			Processor::Processor(Processor * owner, ProcessorID id, ProcessorFunc func){
				pid = id;
				lastchildpid = 0;
				this->func = func;
				parent = owner;
				resources = owner->resources;
				childCount = 0;
				firstLink = 0;
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


			Error initialize(FlowsAPI * api){
				return Processor::loadClass(api);
			}


			Error finalize(FlowsAPI * api){
				return Processor::unloadClass();
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
}

/*
using namespace kq::core::flow;


FlowError IFlowSessionClient::getLocalFactory(kq::core::memory::Pointer<IFactory> &){
	return kErrNotImplemented;
}

class FlowSocket{

};

class FlowSessionServer: public IFlowSessionServer{
	oops::Log l;
	Pointer<IFlowSession> pSession;
	Pointer<IFlowSessionClient> pClient;
	Pointer<IFactory> pClientFactory;

public:
	FlowSessionServer(Pointer<IFlowSession> pSession){
		l.log("FlowSessionServer::FlowSessionServer()\n");
		this->pSession = pSession;
		pClient = pSession->getClient();

		IFlowSessionClient * pC = pClient;
		if(kErrNone != pC->getLocalFactory(pClientFactory)){
			pClientFactory = 0;
		}
	}

	FlowError createSocket(ProcessorType *, kq::core::memory::Pointer<ISocket> & pSocket){
		FlowError ret;
		if(pClientFactory){

		}
		else
			ret = kErrUndefinedProcessorType;
		return ret;
	}

	~FlowSessionServer(){
		l.log("FlowSessionServer::~FlowSessionServer()\n");
	}

};

class FlowSession:public IFlowSession{

	oops::Log l;
	MemoryWorker &mem;
	WeakPointer<FlowSession> pSession;
	WeakPointer<FlowSessionServer> pFlowSessionServer;
	WeakPointer<IFlowSessionClient> pFlowSessionClient;
public:
	kq::core::memory::WeakPointer<IFlowSessionServer> getServer(){return pFlowSessionServer;}
	kq::core::memory::WeakPointer<IFlowSessionClient> getClient(){return pFlowSessionClient;}
	MemoryWorker & getMemoryWorker(){
		return mem;
	}

	WeakPointer<FlowSession> getThis(){
		return pSession;
	}

	FlowSession(WeakPointer<FlowSession> pThis, IFlowSession::FlowsSessionInitOptions * pOptions):  mem(*pOptions->mem), pSession(pThis){
		l.log("FlowSession::FlowSession()\n");
		pFlowSessionClient = pOptions->pClient;
		pFlowSessionServer = kq_core_memory_workerRefCountedClassNew(mem, FlowSessionServer, getThis());
	}

	~FlowSession(){
		l.log("FlowSession::~FlowSession()\n");
	}

	static void destroyFlowSession(void *, RefCounter * counter, void * obj){
		if(obj){
			((FlowSession *)(obj))->~FlowSession();
			free(obj);
		}
		if(counter){
			RefCounter * pCounter = (RefCounter *) counter;
			pCounter->~RefCounter();
			free(pCounter);
		}
	}

	static FlowError createFlowSession(FlowsSessionInitOptions * pOptions, Pointer<IFlowSession> & pUserSession){
		FlowError ret;
		RefCounter * pCounter = (RefCounter *)(*pOptions->mem)(0, sizeof(*pCounter));
		FlowSession * pSession = (FlowSession *)(*pOptions->mem)(0, sizeof(*pSession));
		new (pCounter) RefCounter(pSession, DestructionWorker(DestructionWorkerFunc_workerDelete<FlowSession>, pOptions->mem));
		new (pSession) FlowSession(pCounter, pOptions);
		if(pSession){
			pUserSession = pSession->getThis();
			ret = kErrNone;
			goto done;
		}
		else
			ret = kErrOutOfMemory;

		done:
		return ret;
	}
};

FlowError IFlowSession::createFlowSession(FlowsSessionInitOptions * pOptions, Pointer<IFlowSession> & pUserSession){
	return FlowSession::createFlowSession(pOptions, pUserSession);
}

*/
