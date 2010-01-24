#ifndef MEMORYWORKER_HPP
#define MEMORYWORKER_HPP

#include "core_IntegerTypes.hpp"
#include "core_Worker.hpp"

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


#endif
