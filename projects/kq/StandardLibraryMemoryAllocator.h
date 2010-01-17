#ifndef _STANDARD_LIBRARY_MEMORY_ALLOCATOR_H_
#define _STANDARD_LIBRARY_MEMORY_ALLOCATOR_H_

#include "IntegerTypes.h"
#include "Worker.h"

namespace kore{
	namespace memory{

		class StandardLibraryMemoryAllocator{
			static void * allocator(void * context, void * p, ui64 n);
		public:

			//ui32 m_nBytesAllocated;

			StandardLibraryMemoryAllocator();
			~StandardLibraryMemoryAllocator();			

			void getMemoryWorker(MemoryWorker & MemoryWorker);
		};

	};
};

#endif