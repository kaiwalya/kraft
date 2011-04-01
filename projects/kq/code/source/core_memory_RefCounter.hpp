#ifndef KQ_CORE_MEMORY_REFCounter
#define KQ_CORE_MEMORY_REFCounter

#include "stdio.h"

static int iDepth = 0;

#include "memory.h"
class LogInOut{
	const char * m_pFunction;
	void log(bool bEntry){
		const char * sMark;
		if(bEntry){
			sMark = "{";
		}
		else{
			sMark = "}";
		}
		char * tabs;
		tabs = new char[iDepth + 1];
		memset(tabs, '\t', sizeof(char) * iDepth);
		tabs[iDepth] = 0;

		if(bEntry){
			printf("%s%s%s\n", tabs, m_pFunction, sMark);
		}else{
			printf("%s%s\n", tabs, sMark);
		}
		delete [] tabs;
	}
public:

	void pushdepth(){
		char * tabs;
		tabs = new char[iDepth + 1];
		memset(tabs, '\t', sizeof(char) * iDepth);
		tabs[iDepth] = 0;
		printf("%s", tabs);
		delete [] tabs;
	}

	LogInOut(const char * pFunction):m_pFunction(pFunction){

		log(true);
		iDepth++;
	}
	~LogInOut(){
		iDepth--;
		log(false);
	}
};

#define LOGINOUT LogInOut var##__FUNCTION__(__PRETTY_FUNCTION__);
#define LOGDEPTH var##__FUNCTION__.pushdepth()


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
				virtual ~RefCounter(){
					LOGINOUT;
				}

				static RefCounter nullCounter;

				RefCounter(void * object, DestructionWorker destructionWorker, ui32 count = 0);

				void increment(){
					LOGINOUT;
					++count;
					LOGDEPTH;printf("w %d, s %d\n", count, countWeak);
				};

				void incrementWeak(){
					LOGINOUT;
					++countWeak;
					LOGDEPTH;printf("w %d, s %d\n", count, countWeak);
				}

				void * getObject(){
					return object;
				}
				const void * getObject() const{
					return object;
				};

				void decrement(){
					LOGINOUT;
					--count;
					LOGDEPTH;printf("w %d, s %d\n", count, countWeak);
					if(count == 0){
						if(object){
							void * obj = object;
							object = 0;
							if(countWeak != 0){
								//It is possible that countWeak is one because the object itself
								//is holding on to it. For this we first incrementWeak
								//This assures that "this" - the RefCounter is not deleted during destruction
								incrementWeak();
								destructor(0, obj);
								decrementWeak();
							}
							else{
								destructor(this, object);
							}
						}
					}
				};

				void decrementWeak(){
					LOGINOUT;
					--countWeak;
					LOGDEPTH;printf("w %d, s %d\n", count, countWeak);
					if(countWeak == 0){
						if(count == 0 && this != &nullCounter){
							destructor(this, 0);
						}
					}
				}

			};


			template<typename t> class Pointer;
			template<typename t> class WeakPointer;


			struct Reference{
				RefCounter * c;
				PtrOffset i;

				Reference(){
					c = &RefCounter::nullCounter;
					i = 0;
				}

				Reference(const Reference & other){
					c = other.c;
					i = other.i;
				}

				const Reference & operator = (const Reference & other){
					c = other.c;
					i = other.i;
					return *this;
				}

			};

			template<void (RefCounter::*up)(), void (RefCounter::*down)()>
			class RefHolderBase{

			protected:
				Reference m_ref;

				void swap(const Reference & pOther){
					LOGINOUT;
					(m_ref.c->*up)();
				}

			public:
				RefHolderBase(){
					LOGINOUT;
					(m_ref.c->*up)();
				}

				RefHolderBase(const Reference & ref):m_ref(ref){
					LOGINOUT;
					(m_ref.c->*up)();
				}

				RefHolderBase(const RefHolderBase & other):m_ref(other.m_ref){
					LOGINOUT;
					(m_ref.c->*up)();
				}

				operator Reference & (){
					LOGINOUT;
					return m_ref;
				}

				operator const Reference & () const{
					LOGINOUT;
					return m_ref;
				}

				virtual ~RefHolderBase(){
					LOGINOUT;
					(m_ref.c->*down)();
				}


			};



			template<typename type, void (RefCounter::*up)(), void (RefCounter::*down)()>
			class RefHolder:public RefHolderBase<up, down>{
			public:
				RefHolder(){}
				RefHolder(const Reference & r):RefHolderBase<up, down>(r){}

			};

			template<typename type>
			class WeakPointer: public RefHolder<type, &RefCounter::incrementWeak, &RefCounter::decrementWeak>{
			protected:
			public:
			};

			template<typename type>
			class Pointer: public RefHolder<type, &RefCounter::increment, &RefCounter::decrement>{
			public:
				Pointer(const Reference &r):RefHolder<type, &RefCounter::increment, &RefCounter::decrement>(r){}
				Pointer(){}
			};


			/*
			template<typename t>
			class WeakPointer: public WeakRefHolder<t>{
			};

			template<typename t>
			class Pointer: public StrongRefHolder<t>{
			public:
			};
			*/

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

