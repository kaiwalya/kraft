#include "core_memory_StandardLibraryMemoryAllocator.hpp"
#include "malloc.h"
#include "stdio.h"

using namespace kq::core;
using namespace kq::core::memory;

StandardLibraryMemoryAllocator::StandardLibraryMemoryAllocator(){
	m_nBytesAllocated = 0;
}

StandardLibraryMemoryAllocator::~StandardLibraryMemoryAllocator(){
	printf("%d Bytes Leaked\n", m_nBytesAllocated);
}

void * StandardLibraryMemoryAllocator::allocator(void * context, void * p, ui64 n){
	context;
	StandardLibraryMemoryAllocator * pAllocator = (StandardLibraryMemoryAllocator *)context;

	ui32 & nBytes = pAllocator->m_nBytesAllocated;
	
	if(p){
		nBytes -= (ui32)_msize(p);
		printf("(--,%p,%d)", p, (ui32)_msize(p));					
	}
	

	void * pRet = 0;
	if(p){
		if(n){
			pRet = realloc(p, static_cast<size_t>(n));
		}else{
			free(p);
		}
	}else{
		pRet = malloc(static_cast<size_t>(n));
	}
		
	
	if(pRet){
		nBytes += (ui32)_msize(pRet);
		printf("(++,%p,%d)", pRet, (ui32)_msize(pRet));			
	}


	printf("\n");
	return pRet;
};

void StandardLibraryMemoryAllocator::getMemoryWorker(MemoryWorker & MemoryWorker){
	MemoryWorker.set(this, &(StandardLibraryMemoryAllocator::allocator));
};
