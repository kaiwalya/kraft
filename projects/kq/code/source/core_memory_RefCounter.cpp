#include "core_memory_RefCounter.hpp"
#include "core_IntegerTypes.hpp"

kq::core::memory::RefCounter kq::core::memory::RefCounter::nullCounter;

using namespace kq::core;
using namespace kq::core::memory;

RefCounter::RefCounter(void * object, DestructionWorker destructionWorker, ui32 count):destructor(destructionWorker){

	this->count = count;
	this->countWeak = 0;
	this->object = (ui8 *)object;
}

void kq::core::memory::DestructionWorkerFunc_workerFree(void * worker, RefCounter * pCounter, void * pObject){
	kq::core::memory::MemoryWorker & mem = *((kq::core::memory::MemoryWorker *)worker);
	if(pObject)mem(pObject, 0);
	if(pCounter)mem(pCounter, 0);
};

void kq::core::memory::DestructionWorkerFunc_noOp(void *, RefCounter *, void *){}

/*
void * ProxyNew::operator new(size_t nBytes, Pointer<MemoryWorker> pWorker){
	Pointer<MemoryWorker> pTemp;
	char * pAlloc = (char *)pWorker->operator()(0, nBytes + sizeof(pTemp));
	memcpy(pAlloc, &pTemp, sizeof(pTemp));
	*((Pointer<MemoryWorker> *)(pAlloc)) = pWorker;
	return (pAlloc + sizeof(pTemp));
}

void ProxyNew::operator delete(void * p){
	Pointer<MemoryWorker> pTemp;
	char * pAlloc = (char *)p - sizeof(pTemp);
	memcpy(&pTemp, pAlloc, sizeof(pTemp));
	pTemp->operator()(pAlloc, 0);
}

*/
