#ifndef POOLEDMEMORYALLOCATOR_H_
#define POOLEDMEMORYALLOCATOR_H_

#include "../IntegerTypes.hpp"
#include "MemoryWorker.hpp"

namespace kq{

	namespace core{

		namespace memory{

			class PooledMemoryAllocator{

				struct BlockHeader{
					union{
						BlockHeader ** pPool;
						BlockHeader * pNext;
					};
				};


				kq::core::memory::MemoryWorker & memory;
				kq::core::ui8 * arrMapSizeToPoolArrayIndex;
				
				static const kq::core::ui64 arrSize[];
				static const kq::core::ui8 nSizes;
				static const kq::core::ui32 iBalanceThreshold;
				static const kq::core::ui32 iBalanceToThreshold;

				BlockHeader ** arrPoolArray;
				kq::core::ui32 * arrPoolCount;

				
				
				void balancePool(kq::core::ui8 index, kq::core::ui32 iThreshold);
				void cleanupPool(kq::core::ui8 index);
				void cleanupPools();
				

				void * allocator(void * p, kq::core::ui64 n);
				static void * _allocator(void * context, void * p, kq::core::ui64 n);

				void operator = (PooledMemoryAllocator &){
				}
					
			public:

				
				static const kq::core::ui64 nMaxSize;
				

				PooledMemoryAllocator(MemoryWorker  & memoryMemoryWorker);

				~PooledMemoryAllocator();	

				void getMemoryWorker(MemoryWorker & MemoryWorker);
			};

		};

	};

};

#endif