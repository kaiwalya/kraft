#ifndef POOLEDMEMORYALLOCATOR_H_
#define POOLEDMEMORYALLOCATOR_H_

#include "IntegerTypes.h"
#include "Worker.h"

namespace kore{

	namespace memory{

		class PooledMemoryAllocator{

			struct BlockHeader{
				union{
					BlockHeader ** pPool;
					BlockHeader * pNext;
				};
			};


			MemoryWorker & memory;
			ui8 * arrMapSizeToPoolArrayIndex;
			
			static const ui64 arrSize[];
			static const ui8 nSizes;
			static const ui32 iBalanceThreshold;
			static const ui32 iBalanceToThreshold;

			BlockHeader ** arrPoolArray;
			ui32 * arrPoolCount;

			
			
			void balancePool(ui8 index, ui32 iThreshold);
			void cleanupPool(ui8 index);
			void cleanupPools();
			

			void * allocator(void * p, ui64 n);
			static void * _allocator(void * context, void * p, ui64 n);
				
		public:

			
			static const ui64 nMaxSize;
			

			PooledMemoryAllocator(MemoryWorker  & memoryMemoryWorker);

			~PooledMemoryAllocator();	

			void getMemoryWorker(MemoryWorker & MemoryWorker);
		};

	};

};


#endif