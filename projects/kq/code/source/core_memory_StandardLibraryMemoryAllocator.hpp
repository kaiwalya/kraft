#ifndef _STANDARD_LIBRARY_MEMORY_ALLOCATOR_H_
#define _STANDARD_LIBRARY_MEMORY_ALLOCATOR_H_

#include "core_IntegerTypes.hpp"
#include "core_memory_MemoryWorker.hpp"

namespace kq{

	namespace core{
		namespace memory{

			
			class StandardLibraryMemoryAllocator{
				static void * allocator(void * context, void * p, kq::core::ui64 n);
				
				ui32 m_nBytesAllocated;
				ui32 m_nAllocations;
			public:

				ui32 getCurrentlyAllocatedByteCount();

				StandardLibraryMemoryAllocator();
				~StandardLibraryMemoryAllocator();			

				void getMemoryWorker(kq::core::memory::MemoryWorker & memoryWorker);
			};

		};
	};

};

#endif
