#include "core_memory_PooledMemoryAllocator.hpp"
#include "core_oops.hpp"

//memset memcpy
#include "string.h"
#include "assert.h"

using namespace kq::core;
using namespace kq::core::memory;
using namespace oops;

PooledMemoryAllocator::PooledMemoryAllocator(MemoryWorker  & memoryMemoryWorker):memory(memoryMemoryWorker){

	arrMapSizeToPoolArrayIndex = (ui8 *)memory(0, sizeof(*arrMapSizeToPoolArrayIndex) * (nMaxSize + 1));
	
	ui32 i = 0;
	ui8 index = 0;
	while(i <= nMaxSize){
		while(i <= arrSize[index]){
			arrMapSizeToPoolArrayIndex[i] = index;
			i++;
		}
		index++;
	}
	

	
	arrPoolArray = (BlockHeader **)memory(0, sizeof(BlockHeader *) * nSizes);
	arrPoolCount = (ui32 *)memory(0, sizeof(ui32) * nSizes);
	index = 0;
	while(index < nSizes){
		arrPoolArray[index] = 0;
		arrPoolCount[index] = 0;
		index++;
	}

	
}

PooledMemoryAllocator::~PooledMemoryAllocator(){
	cleanupPools();
	memory(arrPoolArray, 0);
	memory(arrPoolCount, 0);
	memory(arrMapSizeToPoolArrayIndex, 0);
}

void PooledMemoryAllocator::balancePool(ui8 index, ui32 iThreshold){
	//printf("(%d)", arrSize[index]);

	BlockHeader ** pHeader = &arrPoolArray[index];
	ui32 * pCount = &arrPoolCount[index];
	while(*pHeader && (*pCount>iThreshold)){
		BlockHeader * pNext = (*pHeader)->pNext;
		memory(*pHeader, 0);
		*pHeader = pNext;
		(*pCount)--;
	}

}
void PooledMemoryAllocator::cleanupPool(ui8 index){
	balancePool(index, 0);
	if(arrPoolArray[index] || arrPoolCount[index]){
		//printf("Cleanup Failed\n");
		assert(0);
	}
}
void PooledMemoryAllocator::cleanupPools(){		
	ui8 index = 0;
	while(index < nSizes){
		cleanupPool(index);
		index++;
	}
}


void * PooledMemoryAllocator::allocator(void * p, ui64 n){		
	void * pRet = 0;
	const ui64 nBytes = n + sizeof(BlockHeader);
	
	if(p){
		//Free or realloc
		BlockHeader * pBlock = (BlockHeader *)p;
		pBlock--;
		
		BlockHeader ** ppBlock = pBlock->pPool;
		
		if(n){
			//realloc				
			BlockHeader ** ppBlockNew = 0;
			i16 indexNew = -1;
			if(nBytes <= nMaxSize){
				indexNew = arrMapSizeToPoolArrayIndex[nBytes];
				ppBlockNew = &(arrPoolArray[indexNew]);
			}

			if(ppBlock == 0){
				//source is unpooled

				if(ppBlockNew){
					//unpooled to pooled

					//Since we do not have size information 
					//for the initially unpooled location,
					//We will need to realloc the old block
					//to the size of a pool.
					pBlock = (BlockHeader *)memory(pBlock, arrSize[indexNew]);
					if(!pBlock){					
						cleanupPools();
						pBlock = (BlockHeader *)memory(pBlock, arrSize[indexNew]);
						if(!pBlock){
							throw OutOfMemoryException();
						}
					}
					
					//and add it to the pool
					//If we had size we could have used an already pooled
					//block						
					pBlock->pPool = ppBlockNew;
					//printf("(UP ->  P)");
				}else{
					//Note the Header is also copied by this call
					pBlock = (BlockHeader *)memory(pBlock, nBytes);
					if(!pBlock){					
						cleanupPools();
						pBlock = (BlockHeader *)memory(pBlock, nBytes);
						if(!pBlock){
							throw OutOfMemoryException();
						}
					}
					//printf("(UP -> UP)");
				}
				pRet = (pBlock + 1);
			}
			else{
				//source is pooled
				if(ppBlock == ppBlockNew){						
					//dest is the same pool
					//no copy necessry, since data is in the same place
					pRet = p;
					//printf("( P -> *P)");
				}else{

					//Dest is in a different pool or dest is unpooled
					pRet = allocator(0, n);
					
					BlockHeader * pBlockNew = (BlockHeader *)pRet;
					pBlockNew--;
					
					//We want to copy the user data, 
					//note that all blocks allocated through this call
					//have a block header at the top, so we sutract that
					//Also note that p and pRet are client pointers,
					//Not the header pointers, so we need to subtract the header

					//Get the smaller size for a memcpy
					size_t nBytesToCopy = (size_t)n;
					size_t nSourceClientSize = (size_t)arrSize[ppBlock - arrPoolArray] - sizeof(BlockHeader);
					if(nSourceClientSize < nBytesToCopy){
						nBytesToCopy = nSourceClientSize;
					}
					//nBytesToCopy = 4;
					//if(nBytesToCopy + sizeof(BlockHeader) > pBlock->iSize || nBytesToCopy + sizeof(BlockHeader) > pBlockNew->iSize){
					//	_asm int 3;
					//}
					memcpy(pRet, p, nBytesToCopy);

					//Free up the old memory
					allocator(p, 0);
					if(ppBlockNew){
						//printf("( P ->  P)");
					}else{
						//printf("( P -> UP)");
					}

				}					
			};
		}
		else{
			//free				
			if(ppBlock){
				pBlock->pNext = *ppBlock;
				*ppBlock = pBlock;
				ui8 index = (ui8)(ppBlock - arrPoolArray);
				arrPoolCount[index]++;					
				memset(p, 0xFF, (size_t)arrSize[index] - sizeof(*pBlock));
				if(arrPoolCount[index] > iBalanceThreshold){
					balancePool(index, iBalanceToThreshold);
				}
			}else{
				memory(pBlock, 0);
			};
		}
	}else{
		//Alloc
		if(n != 0){
			if(nBytes <= nMaxSize){
				
				const ui8 index = arrMapSizeToPoolArrayIndex[nBytes];

				BlockHeader ** ppBlock = &(arrPoolArray[index]);

				if(*ppBlock){
					//Empty blocks exist
					BlockHeader * pBlock = *ppBlock;
					*ppBlock = (*ppBlock)->pNext;					
					pBlock->pPool = ppBlock;
					arrPoolCount[index]--;
					
					pRet = pBlock + 1;
				}else{
					//Need to allocate blocks;
					BlockHeader * pBlock;
					ui8 i = 0;
					do{
						pBlock = (BlockHeader *)(memory(0, arrSize[index]));
						if(arrSize[index] < nBytes ){
							assert(0);
						}

						if(!pBlock){
							//Couldnt allocate pool entry

							//check if there are some blocks in pool
							if(*ppBlock){
								//some blocks are allocated
								//thats enough for now.
								//break;
							}else{
								//try removing other unused pools
								cleanupPools();
								pBlock = (BlockHeader *)(memory(0, arrSize[index]));
								if(!pBlock){
									throw OutOfMemoryException();
								}
							}
						}
						memset(pBlock, 0xAA, (size_t)arrSize[index]);						
						pBlock->pPool = ppBlock;
						pBlock->pNext = *ppBlock;
						*ppBlock = pBlock;							
						arrPoolCount[index]++;					
						i++;
					}while(arrPoolCount[index] <= iBalanceThreshold && i);

					pRet = allocator(p, n);
				};

			}
			else{
				//Allocate unpooled block
				BlockHeader *pBlock = 0;
				pBlock = (BlockHeader *)(memory(0, nBytes));
				if(!pBlock){
					cleanupPools();
					pBlock = (BlockHeader *)(memory(0, nBytes));
					if(!pBlock){
						throw OutOfMemoryException();
					}
				}
				pBlock->pPool = 0;
				pRet = (pBlock + 1);
			};

		};//if n!= 0
	};

	/*
	if(pRet){
		BlockHeader * pBlock = (BlockHeader *)pRet;
		pBlock--;
	}
	*/
	

	return pRet;
};

void * PooledMemoryAllocator::_allocator(void * context, void * p, ui64 n){
	//printf("(Req: %p, %16d)", p, (ui32)n);
	void * pRet = ((PooledMemoryAllocator *)context)->allocator(p,n);
	//printf("(%p)\n", pRet);
	return pRet;
};

void PooledMemoryAllocator::getMemoryWorker(MemoryWorker & MemoryWorker){
	MemoryWorker.set(this, &_allocator);
};


const ui64 PooledMemoryAllocator::arrSize[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
const ui8 PooledMemoryAllocator::nSizes = sizeof(arrSize)/sizeof(arrSize[0]);
const ui64 PooledMemoryAllocator::nMaxSize = arrSize[nSizes - 1];
const ui32 PooledMemoryAllocator::iBalanceThreshold = 512;
const ui32 PooledMemoryAllocator::iBalanceToThreshold = 256;

