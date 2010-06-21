#ifndef KQ_CORE_MEMORY_REFCounter
#define KQ_CORE_MEMORY_REFCounter

#include "core_memory_MemoryWorker.hpp"

namespace kq{
	namespace core{
		namespace memory{

			class RefCounter;


			class DestructionWorker:public kq::core::Worker<void *, void (*)(void * , RefCounter *, void *)>{
			public:
				DestructionWorker(void (*pfnWorker)(void * , RefCounter *, void *), void * pContext = 0){
					set(pContext, pfnWorker);
				};
			public:
				void operator()(RefCounter * pCounter, void * pObject){
					return (*getWorkerFunction())(getWorkerContext(), pCounter, pObject);
				};
			};

			void DestructionWorkerFunc_noOp(void *, RefCounter *, void *);

			template<typename classname>
			void DestructionWorkerFunc_delete(void *, RefCounter * pCounter, void * pObject){
				delete ((classname *)pObject);
				delete (pCounter);
			};

			void DestructionWorkerFunc_workerFree(void * worker, RefCounter * pCounter, void * pObject);		

			template<typename classname>
			void DestructionWorkerFunc_workerDelete(void * worker, RefCounter * pCounter, void * pObject){				
				((classname *)pObject)->~classname();
				pCounter->~RefCounter();
				DestructionWorkerFunc_workerFree(worker, pCounter, pObject);
			};



			class RefCounter{
			protected:
				void * object;				
				ui32 count;

				DestructionWorker destructor;

				RefCounter():object(0), count(0), destructor(DestructionWorkerFunc_noOp){};
								
			public:
				virtual ~RefCounter(){}

				static RefCounter nullCounter;

				RefCounter(void * object, DestructionWorker destructionWorker, ui32 count = 0);

				ui32 increment(){
					return ++count;
				};

				void * getObject(){
					return object;
				}
				const void * getObject() const{
					return object;
				};

				ui32 decrement(){
					int iRet = --count;
					if(iRet == 0){						
						if(object){
							destructor(this, object);
						}
					}
					return iRet;
				};

			};

			template<typename t>
			class Pointer{
			public:
				RefCounter * m_pRefCounter;
				t* m_pBufferedObject;

				void setReference(RefCounter * pRefCounter){
					m_pRefCounter = pRefCounter;
					m_pBufferedObject = reinterpret_cast<t *>(pRefCounter->getObject());
				};

				void attach(RefCounter * pRefCounter){
					setReference(pRefCounter);
					m_pRefCounter->increment();       
				};
			   
				void detach(){
					m_pRefCounter->decrement();
					setReference(&kq::core::memory::RefCounter::nullCounter);
				}	
			public:

				Pointer(RefCounter * pRefCounter){
					if(pRefCounter){
						attach(pRefCounter);
					}else{
						attach(&kq::core::memory::RefCounter::nullCounter);
					}					
				};

				Pointer(const Pointer<t> & pointer){
					attach(pointer.m_pRefCounter);
				};

				Pointer(){
					attach(&kq::core::memory::RefCounter::nullCounter);
				}

				~Pointer(){
					detach();
				}

				//We do not want to use a reference here, else "p = p->next" will fail when refcount of p == 1;
				Pointer<t> & operator = (const Pointer<t> oprand){
					if(m_pRefCounter->getObject() != oprand.m_pRefCounter->getObject()){
						detach();
						attach(oprand.m_pRefCounter);           
					}
					return *this;
				};

				operator Pointer<const t>()const{

					Pointer<const t> ret(m_pRefCounter);
					return ret;
				}

				bool operator == (const Pointer<t> & oprand) const{
					return (oprand.m_pRefCounter->getObject() == m_pBufferedObject);

				}

				bool operator != (const Pointer<t> & oprand) const{
					return (oprand.m_pRefCounter->object != m_pBufferedObject);
				}

				t * operator ->()const {
					if(!m_pBufferedObject){
						_asm int 3;
					}
					return (t *)(m_pBufferedObject);
				};

				
				t & operator *() const{
					return *(m_pBufferedObject);
				}
				

				operator bool (){
					return m_pBufferedObject != 0;
				}


				template<typename t2>
				Pointer<t2> cast(){
					t * pt1 = 0;
					t2 * pt2;
					pt2 = pt1;

					return Pointer<t2 *>(m_pRefCounter);
				}

				template<typename t2>
				Pointer<t2> castStatic(){

					t * pt1 = 0;
					t2 * pt2;
					pt2 = static_cast<t2 *>(pt1);

					return Pointer<t2>(m_pRefCounter);
				}

				
				template<typename t2>
				Pointer<t2> castDynamic(){
					
					t * pt1 = 0;
					t2 * pt2;
					pt2 = dynamic_cast<t2 *>(pt1);

					return Pointer<t2>(m_pRefCounter);
				}

				
				template<typename t2>
				Pointer<t2> castReinterpret(){
					
					t * pt1 = 0;
					t2 * pt2;
					pt2 = reinterpret_cast<t2 *>(pt1);

					return Pointer<t2>(m_pRefCounter);
				}
										
			};

		};
	};
};


#define kq_core_memory_workerRefCountedMalloc(memworker, size) (kq_core_memory_workerNew(memworker,kq::core::memory::RefCounter,(memworker(0, size),kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerFree, &memworker))))
#define kq_core_memory_workerRefCountedBasicNew(memworker, classname) (kq_core_memory_workerNew(memworker,kq::core::memory::RefCounter,(memworker(0, sizeof(classname)),kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerFree, &memworker))))
#define kq_core_memory_workerRefCountedClassNew(memworker, classname, ...) (kq_core_memory_workerNew(memworker, kq::core::memory::RefCounter, (kq_core_memory_workerNew(memworker, classname, (__VA_ARGS__)), kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerDelete<classname>, &memworker))))


#endif