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
			void DestructionWorkerFunc_delete(void *, RefCounter * pCounter, void * pObject);

			void DestructionWorkerFunc_workerFree(void * worker, RefCounter * pCounter, void * pObject);		

			template<typename classname>
			void DestructionWorkerFunc_workerDelete(void * worker, RefCounter * pCounter, void * pObject);


			class RefCounter{
			protected:
				void * object;				
				ui32 count;
				ui32 countWeak;

				DestructionWorker destructor;

				RefCounter():object(0), count(0),countWeak(0), destructor(DestructionWorkerFunc_noOp){};
								
			public:
				virtual ~RefCounter(){}

				static RefCounter nullCounter;

				RefCounter(void * object, DestructionWorker destructionWorker, ui32 count = 0);

				ui32 increment(){
					return ++count;
				};

				ui32 incrementWeak(){
					return ++countWeak;

				}

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
							if(countWeak != 0){
								//It is possible that countWeak is one because the object itself
								//is holding on to it. For this we first incrementWeak
								//This assures that "this" - the RefCounter is not deleted during destruction
								incrementWeak();
								destructor(0, object);
								decrementWeak();
							}
							else{
								destructor(this, object);
							}
							object = 0;
						}
					}

					return iRet;
				};

				ui32 decrementWeak(){
					int iRet = --countWeak;
					if(iRet == 0){
						if(count == 0 && this != &nullCounter){
							destructor(this, 0);
						}
					}

					return iRet;
				}

			};


			template<typename t> class Pointer;
			template<typename t> class WeakPointer;

			template<typename type>
			class RefHolder{
			private:
				RefCounter * m_pRefCounter;
				PtrOffset m_iOffset;

				friend class WeakPointer<type>;
				friend class Pointer<type>;
			};

			template<typename t>
			class WeakPointer: public RefHolder<t>{


			public:

				void setReference(RefCounter * pRefCounter, PtrOffset offset){
					RefHolder<t>::m_pRefCounter = pRefCounter;
					RefHolder<t>::m_iOffset = offset;
				};

				void attach(RefCounter * pRefCounter, PtrOffset offset = 0){
					setReference(pRefCounter, offset);
					RefHolder<t>::m_pRefCounter->incrementWeak();
				};

				void detach(){
					RefHolder<t>::m_pRefCounter->decrementWeak();
					setReference(&kq::core::memory::RefCounter::nullCounter, 0);
				}

				WeakPointer(RefCounter * pRefCounter, PtrOffset offset){
					if(pRefCounter){
						attach(pRefCounter, offset);
					}else{
						attach(&kq::core::memory::RefCounter::nullCounter);
					}
				};

				ui8 * location(){
					((char *)RefHolder<t>::m_pRefCounter->getObject()) + RefHolder<t>::m_iOffset;
				}
			public:

				WeakPointer(RefCounter * pRefCounter){
					if(pRefCounter){
						attach(pRefCounter);
					}else{
						attach(&kq::core::memory::RefCounter::nullCounter);
					}
				};

				WeakPointer(const RefHolder<t> & pointer){
					attach(pointer.RefHolder<t>::m_pRefCounter, pointer.RefHolder<t>::m_iOffset);
				};


				WeakPointer(const WeakPointer<t> & pointer){
					attach(pointer.RefHolder<t>::m_pRefCounter, pointer.RefHolder<t>::m_iOffset);
				};

				WeakPointer(){
					attach(&kq::core::memory::RefCounter::nullCounter);
				}

				~WeakPointer(){
					detach();
				}

				//We do not want to use a reference here, else "p = p->next" will fail when refcount of p == 1;
				WeakPointer<t> & operator = (const RefHolder<t> oprand){
					if(RefHolder<t>::m_pRefCounter != oprand.RefHolder<t>::m_pRefCounter){
						detach();
						attach(oprand.RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					else if(RefHolder<t>::m_iOffset != oprand.RefHolder<t>::m_iOffset)
					{
						setReference(RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					return *this;
				};

				//We do not want to use a reference here, else "p = p->next" will fail when refcount of p == 1;
				WeakPointer<t> & operator = (const WeakPointer<t> oprand){
					if(RefHolder<t>::m_pRefCounter != oprand.RefHolder<t>::m_pRefCounter){
						detach();
						attach(oprand.RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					else if(RefHolder<t>::m_iOffset != oprand.RefHolder<t>::m_iOffset)
					{
						setReference(RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					return *this;
				};

				operator WeakPointer<const t>()const{

					WeakPointer<const t> ret(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
					return ret;
				}

				bool operator == (const WeakPointer<t> & oprand) const{
					return (oprand.RefHolder<t>::m_pRefCounter == RefHolder<t>::m_pRefCounter && oprand.RefHolder<t>::m_iOffset == RefHolder<t>::m_iOffset);

				}

				bool operator != (const WeakPointer<t> & oprand) const{
					return (oprand.RefHolder<t>::m_pRefCounter != RefHolder<t>::m_pRefCounter || oprand.RefHolder<t>::m_iOffset != RefHolder<t>::m_iOffset);
				}

				t * operator ->()const {
					void * pRet = location();
					if(!pRet){
						//_asm int 3;
					}
					return (t *)(pRet);
				};



				t & operator *() const{
					return *((t*)location());
				}

				WeakPointer<t> operator +(ArrayIndex index){
					return WeakPointer(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset + (index * sizeof(t)));
				}

				WeakPointer<t> operator -(ArrayIndex index){
					return WeakPointer(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset - (index * sizeof(t)));
				}

				WeakPointer<t> & operator ++(){
					RefHolder<t>::m_iOffset += sizeof(t);
					return *this;
				}

				//There needs to be an int here for this to be a postfix operator overload
				WeakPointer<t> & operator ++(int i){
					i++;
					RefHolder<t>::m_iOffset += sizeof(t);
					return *this;
				}

				WeakPointer<t> & operator --(){
					RefHolder<t>::m_iOffset -= sizeof(t);
				}

				WeakPointer<t> & operator += (ArrayIndex index){
					RefHolder<t>::m_iOffset += (index * sizeof(t));
					return *this;
				}

				WeakPointer<t> & operator -= (ArrayIndex index){
					RefHolder<t>::m_iOffset -= (index * sizeof(t));
					return *this;
				}

				bool operator > (WeakPointer<t> & other){
					return (location() > other.location());
				}

				bool operator < (WeakPointer<t> & other){
					return (location() < other.location());
				}

				bool operator >= (WeakPointer<t> & other){
					return (location() >= other.location());
				}

				bool operator <= (WeakPointer<t> & other){
					return (location() <= other.location());
				}


				t & operator [](ArrayIndex index){
					return *(((t*)location()) + index);
				}


				//This check should fail if the object was deallocated
				operator bool (){
					return (RefHolder<t>::m_pRefCounter->getObject()) != 0;
				}


				template<typename t2>
				WeakPointer<t2> cast(){
					t * pt1 = 0;
					t2 * pt2;
					pt2 = pt1;

					return WeakPointer<t2 *>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
				}

				template<typename t2>
				WeakPointer<t2> castStatic(){

					t * pt1 = 0;
					t2 * pt2;
					pt2 = static_cast<t2 *>(pt1);

					return WeakPointer<t2>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
				}


				template<typename t2>
				WeakPointer<t2> castDynamic(){

					t * pt1 = (t*)location();
					t2 * pt2 = dynamic_cast<t2 *>(pt1);
					if(pt2){
						return WeakPointer<t2>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
					}
					else{
						return WeakPointer<t2>();
					}

				}


				template<typename t2>
				WeakPointer<t2> castReinterpret(){

					t * pt1 = 0;
					t2 * pt2;
					pt2 = reinterpret_cast<t2 *>(pt1);

					return WeakPointer<t2>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
				}

			};

			template<typename t>
			class Pointer: public RefHolder<t>{
			public:
				t* m_pBufferedObject;

				void setReference(RefCounter * pRefCounter, PtrOffset offset){
					RefHolder<t>::m_pRefCounter = pRefCounter;
					m_pBufferedObject = reinterpret_cast<t *>((PtrGranular)pRefCounter->getObject() + offset);
					RefHolder<t>::m_iOffset = offset;
				};

				void attach(RefCounter * pRefCounter, PtrOffset offset = 0){
					setReference(pRefCounter, offset);
					RefHolder<t>::m_pRefCounter->increment();
				};
			   
				void detach(){
					RefHolder<t>::m_pRefCounter->decrement();
					setReference(&kq::core::memory::RefCounter::nullCounter, 0);
				}

				Pointer(RefCounter * pRefCounter, PtrOffset offset){
					if(pRefCounter){
						attach(pRefCounter, offset);
					}else{
						attach(&kq::core::memory::RefCounter::nullCounter);
					}					
				};

			public:

				Pointer(RefCounter * pRefCounter){
					if(pRefCounter){
						attach(pRefCounter);
					}else{
						attach(&kq::core::memory::RefCounter::nullCounter);
					}					
				};

				/*
				Pointer(const RefCounted & obj){
					attach(obj.m_pRef, 0);
				}
				*/

				Pointer(const RefHolder<t> & pointer){
					if(pointer.RefHolder<t>::m_pRefCounter->getObject()){
						attach(pointer.RefHolder<t>::m_pRefCounter, pointer.RefHolder<t>::m_iOffset);
					}
					else{
						attach(&kq::core::memory::RefCounter::nullCounter);
					}
				};

				Pointer(const Pointer<t> & pointer){
					attach(pointer.RefHolder<t>::m_pRefCounter, pointer.RefHolder<t>::m_iOffset);
				};

				Pointer(){
					attach(&kq::core::memory::RefCounter::nullCounter);
				}

				~Pointer(){
					detach();
				}

				//We do not want to use a reference here, else "p = p->next" will fail when refcount of p == 1;
				Pointer<t> & operator = (const Pointer<t> oprand){
					if(RefHolder<t>::m_pRefCounter != oprand.RefHolder<t>::m_pRefCounter){
						detach();
						attach(oprand.RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					else if(RefHolder<t>::m_iOffset != oprand.RefHolder<t>::m_iOffset)
					{
						setReference(RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					return *this;
				};


				//We do not want to use a reference here, else "p = p->next" will fail when refcount of p == 1;
				Pointer<t> & operator = (const RefHolder<t> oprand){
					if(RefHolder<t>::m_pRefCounter != oprand.RefHolder<t>::m_pRefCounter){
						detach();
						attach(oprand.RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					else if(RefHolder<t>::m_iOffset != oprand.RefHolder<t>::m_iOffset)
					{
						setReference(RefHolder<t>::m_pRefCounter, oprand.RefHolder<t>::m_iOffset);
					}
					return *this;
				};


				operator Pointer<const t>()const{

					Pointer<const t> ret(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
					return ret;
				}

				bool operator == (const Pointer<t> & oprand) const{
					return ( oprand.m_pBufferedObject == m_pBufferedObject);

				}

				bool operator != (const Pointer<t> & oprand) const{
					return (oprand.m_pBufferedObject != m_pBufferedObject);
				}

				t * operator ->()const {
					if(!m_pBufferedObject){
						//_asm int 3;
					}
					return (t *)(m_pBufferedObject);
				};

				
				
				t & operator *() const{
					return *(m_pBufferedObject);
				}
				
				Pointer<t> operator +(ArrayIndex index){
					return Pointer(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset + (index * sizeof(t)));
				}

				Pointer<t> operator -(ArrayIndex index){
					return Pointer(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset - (index * sizeof(t)));
				}

				Pointer<t> & operator ++(){
					m_pBufferedObject++;
					RefHolder<t>::m_iOffset += sizeof(t);
					return *this;
				}

				//There needs to be an int here for this to be a postfix operator overload
				Pointer<t> & operator ++(int i){

					i++;
					m_pBufferedObject++;
					RefHolder<t>::m_iOffset += sizeof(t);
					return *this;
				}

				Pointer<t> & operator --(){
					m_pBufferedObject--;
					RefHolder<t>::m_iOffset -= sizeof(t);
				}

				Pointer<t> & operator += (ArrayIndex index){
					m_pBufferedObject += index;
					RefHolder<t>::m_iOffset += (index * sizeof(t));
					return *this;
				}
				
				Pointer<t> & operator -= (ArrayIndex index){
					m_pBufferedObject -= index;
					RefHolder<t>::m_iOffset -= (index * sizeof(t));
					return *this;
				}

				bool operator > (Pointer<t> & other){
					return (m_pBufferedObject > other.m_pBufferedObject);
				}
			
				bool operator < (Pointer<t> & other){
					return (m_pBufferedObject < other.m_pBufferedObject);
				}
			
				bool operator >= (Pointer<t> & other){
					return (m_pBufferedObject >= other.m_pBufferedObject);
				}
			
				bool operator <= (Pointer<t> & other){
					return (m_pBufferedObject <= other.m_pBufferedObject);
				}
			

				t & operator [](ArrayIndex index){
					return *(m_pBufferedObject + index);
				}


				operator bool (){
					return m_pBufferedObject != 0;
				}

				template<typename t2>
				Pointer<t2> cast(){
					t * pt1 = 0;
					t2 * pt2;
					pt2 = pt1;

					return Pointer<t2 *>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
				}

				template<typename t2>
				Pointer<t2> castStatic(){

					t * pt1 = 0;
					t2 * pt2;
					pt2 = static_cast<t2 *>(pt1);

					return Pointer<t2>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
				}

				
				template<typename t2>
				Pointer<t2> castDynamic(){
					
					t * pt1 = m_pBufferedObject;
					t2 * pt2 = dynamic_cast<t2 *>(pt1);
					if(pt2){
						return Pointer<t2>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
					}
					else{
						return Pointer<t2>();
					}
					
				}

				
				template<typename t2>
				Pointer<t2> castReinterpret(){
					
					t * pt1 = 0;
					t2 * pt2;
					pt2 = reinterpret_cast<t2 *>(pt1);

					return Pointer<t2>(RefHolder<t>::m_pRefCounter, RefHolder<t>::m_iOffset);
				}
										
			};


			template<typename classname>
			void DestructionWorkerFunc_workerArrayDelete(void * worker, RefCounter * pCounter, void * pObject){
				kq::core::memory::MemoryWorker & mem = *((kq::core::memory::MemoryWorker *)worker);
				if(pObject){
					mem.destroyArray<classname>((classname *)pObject);
				}
				if(pCounter){
					pCounter->~RefCounter();
					mem(pCounter, 0);
				}
			}

			template<typename classname>
			void DestructionWorkerFunc_workerDelete(void * worker, RefCounter * pCounter, void * pObject)
			{				
				if(pObject)((classname *)pObject)->~classname();
				if(pCounter)pCounter->~RefCounter();
				DestructionWorkerFunc_workerFree(worker, pCounter, pObject);
			}

			
			template<typename classname>
			void DestructionWorkerFunc_delete(void *, RefCounter * pCounter, void * pObject){
				if(pObject)	delete ((classname *)pObject);
				if(pCounter) delete (pCounter);
			};

		};
	};
};


#define kq_core_memory_workerRefCountedMalloc(memworker, size)				(kq_core_memory_workerNew(memworker, kq::core::memory::RefCounter, (memworker(0, size),												kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerFree,						&memworker))))
#define kq_core_memory_workerRefCountedBasicNew(memworker, classname)		(kq_core_memory_workerNew(memworker, kq::core::memory::RefCounter, (memworker(0, sizeof(classname)),								kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerFree,						&memworker))))
#define kq_core_memory_workerRefCountedClassNew(memworker, classname, ...)	(kq_core_memory_workerNew(memworker, kq::core::memory::RefCounter, (kq_core_memory_workerNew(memworker, classname, (__VA_ARGS__)),	kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerDelete<classname>,		&memworker))))
#define kq_core_memory_workerRefCountedArrayNew(memworker, classname, n)	(kq_core_memory_workerNew(memworker, kq::core::memory::RefCounter, (kq_core_memory_workerArrayNew(memworker, classname, n),			kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerArrayDelete<classname>,	&memworker))))

#define kq_core_memory_workerRefCountedObjectNew(pThis, memworker, classname, x) { \
		classname * pObject = (classname *)memworker(0, sizeof(classname)); \
		kq::core::memory::RefCounter * pCounter = (kq::core::memory::RefCounter *)memworker(0, sizeof(kq::core::memory::RefCounter));	\
		new (pCounter) kq::core::memory::RefCounter(pObject, kq::core::memory::DestructionWorker(kq::core::memory::DestructionWorkerFunc_workerDelete<classname>, &memworker)); \
		new (pObject) classname  x; \
		pThis = kq::core::memory::Pointer<classname> (pCounter); \
	}
//, __VA_ARGS__
#endif

