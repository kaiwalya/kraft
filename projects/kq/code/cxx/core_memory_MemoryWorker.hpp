#ifndef MEMORYWORKER_HPP
#define MEMORYWORKER_HPP

#include "core_IntegerTypes.hpp"
#include "core_Worker.hpp"
#include <new>

namespace kq{
	namespace core{
		namespace memory{

			typedef kq::core::i32 ArrayIndex;
			typedef kq::core::i32 PtrOffset;
			typedef kq::core::ui8 * PtrGranular;

			class MemoryWorker:public kq::core::Worker<void *, void *(*)(void *, void *, kq::core::ui64)>{								
			protected:
				struct ArrayHeader{
					ArrayIndex nElements;
				};
			public:
				void * operator()(void * p, kq::core::ui64 n){
					return (*getWorkerFunction())(getWorkerContext(), p, n);
				};

				template<typename t>
				t * createArray(ArrayIndex nElements){
					void * pAlloc = (*this)(0, nElements * sizeof(t) + sizeof(ArrayHeader));
					if(pAlloc){
						ArrayHeader * pHdr = (ArrayHeader *)pAlloc;
						pHdr->nElements = nElements;
						pHdr++;

						t * obj = (t*)pHdr;
						ArrayIndex iElement = 0;
						while(iElement < nElements){
							new (obj + iElement) t();
							iElement++;
						}
						return obj;
					}
					return 0;
				};

				template<typename t>
				void destroyArray(t * pTarget){
					ArrayHeader * pHdr = (ArrayHeader *)pTarget;
					pHdr--;
					ArrayIndex i = pHdr->nElements - 1;
					while(i >= 0){
						(pTarget + i)->~t();
						i--;
					}
					(*this)(pHdr, 0);
				}
			};

		};

	};
};

#define kq_core_memory_workerNew(memworker, classname, ...) (new (memworker(0, sizeof(classname))) classname __VA_ARGS__)
#define kq_core_memory_workerDelete(memworker, classname, obj) (obj->~classname());(memworker(obj, 0))
#define kq_core_memory_workerArrayNew(memworker, classname, n) (memworker.createArray<classname>(n))
#define kq_core_memory_workerArrayDelete(memworker, classname, obj) (memworker.destroyArray<classname>(obj))



#endif
