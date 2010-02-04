#ifndef KQ_CORE_MEMORY_REFCounter
#define KQ_CORE_MEMORY_REFCounter

#include "core_memory_MemoryWorker.hpp"

namespace kq{
	namespace core{
		namespace memory{
			class RefCounter{				
			public:
				void * object;
				ui32 count;
				bool (*AtLast)(RefCounter *);

				static RefCounter nullCounter;

				RefCounter(void * object = 0, ui32 count = 0, bool (*AtLast)(RefCounter *) = 0);
				RefCounter(void * object, bool (*AtLast)(RefCounter *));

			};

			template<typename t>
			class Pointer{
				RefCounter * m_pRefCounter;
				t* m_pBufferedObject;

				void setReference(RefCounter * pRefCounter){
					m_pRefCounter = pRefCounter;
					m_pBufferedObject = reinterpret_cast<t *>(pRefCounter->object);
				};

				void attach(RefCounter * pRefCounter){
					setReference(pRefCounter);

					m_pRefCounter->count++;       
				};
			   
				void detach(){
					m_pRefCounter->count--;
					if(!m_pRefCounter->count && m_pRefCounter->object){
						if( !(m_pRefCounter->AtLast) || !(*(m_pRefCounter->AtLast))(m_pRefCounter) ){
							delete (t *)m_pRefCounter->object;
							delete m_pRefCounter;
						}
					}
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

				Pointer<t> & operator = (const Pointer<t> oprand){
					if(m_pRefCounter->object != oprand.m_pRefCounter->object){
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
					return (oprand.m_pRefCount->object == m_pBufferedObject);

				}

				bool operator != (const Pointer<t> & oprand) const{
					return (oprand.m_pRefCount->object != m_pBufferedObject);
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