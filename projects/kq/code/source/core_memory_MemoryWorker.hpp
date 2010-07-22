#ifndef MEMORYWORKER_HPP
#define MEMORYWORKER_HPP

#include "core_IntegerTypes.hpp"
#include "core_Worker.hpp"
#include <new>

namespace kq{
	namespace core{
		namespace memory{
			class MemoryWorker:public kq::core::Worker<void *, void *(*)(void *, void *, kq::core::ui64)>{								
			public:
				void * operator()(void * p, kq::core::ui64 n){
					return (*getWorkerFunction())(getWorkerContext(), p, n);
				};
			};

		};

	};
};

#define kq_core_memory_workerNew(memworker, classname, ...) (new (memworker(0, sizeof(classname))) classname __VA_ARGS__)
#define kq_core_memory_workerDelete(memworker, classname, obj) (obj->~classname());(memworker(obj, 0))



#endif
