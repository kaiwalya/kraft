#include "StandardLibraryMemoryAllocator.h"
#include "malloc.h"

using namespace kore::memory;

StandardLibraryMemoryAllocator::StandardLibraryMemoryAllocator(){
	//m_nBytesAllocated = 0;
}

StandardLibraryMemoryAllocator::~StandardLibraryMemoryAllocator(){
	//printf("%d Bytes Leaked\n", m_nBytesAllocated);
}

void * StandardLibraryMemoryAllocator::allocator(void * context, void * p, ui64 n){
	StandardLibraryMemoryAllocator * pAllocator = (StandardLibraryMemoryAllocator *)context;

	/*
	if(p){
		pAllocator->m_nBytesAllocated -= (ui32)_msize(p);
		//printf("(--,%p,%d)", p, (ui32)_msize(p));					
	}
	*/

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
		
	/*
	if(pRet){
		pAllocator->m_nBytesAllocated += (ui32)_msize(pRet);
		//printf("(++,%p,%d)", pRet, (ui32)_msize(pRet));			
	}
	*/

	return pRet;
};

void StandardLibraryMemoryAllocator::getMemoryWorker(MemoryWorker & MemoryWorker){
	MemoryWorker.set(this, &(StandardLibraryMemoryAllocator::allocator));
};
