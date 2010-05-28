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

			static void DestructionWorkerFunc_noOp(void *, RefCounter *, void *){

			};

			template<typename classname>
			static void DestructionWorkerFunc_delete(void *, RefCounter * pCounter, void * pObject){
				delete ((classname *)pObject);
				delete (pCounter);
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

				RefCounter(void * object, DestructionWorker & worker, ui32 count = 0);

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
					attach(pRefCounter);
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

				Pointer<t> & operator = (const Pointer<t> & oprand){
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

				operator bool (){
					return m_pRefCounter->object != 0;
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

			/*
			class ProxyNew{
			public:
				void * operator new(size_t nBytes, Pointer<MemoryWorker> pWorker);

				void operator delete(void *);
			};
			*/
		};
	};
};

#endif