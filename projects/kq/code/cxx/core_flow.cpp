#include "core_flow.hpp"
#include "memory"
#include "malloc.h"
#include "core_memory.hpp"
#include "core_oops.hpp"

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;
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
