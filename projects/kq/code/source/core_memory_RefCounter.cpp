#include "core_memory_RefCounter.hpp"
#include "core_IntegerTypes.hpp"

#include "string.h"

kq::core::memory::RefCounter kq::core::memory::RefCounter::nullCounter;


using namespace kq::core;
using namespace kq::core::memory;

RefCounter::RefCounter(void * object, DestructionWorker & destrutionWorker, ui32 count):destructor(destrutionWorker){

	this->count = count;
	this->object = object;
	
}

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