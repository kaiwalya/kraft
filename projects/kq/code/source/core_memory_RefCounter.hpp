#ifndef KQ_CORE_MEMORY_REFCounter
#define KQ_CORE_MEMORY_REFCounter

#include "stdio.h"

static int iDepth = 0;

#include "memory.h"
class LogInOut{
	const void * m_pThis;
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
			printf("%s[%p]%s%s\n", tabs, m_pThis, m_pFunction, sMark);
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

	LogInOut(const void * ptr, const char * pFunction):m_pFunction(pFunction), m_pThis(ptr){

		log(true);
		iDepth++;
	}
	~LogInOut(){
		iDepth--;
		log(false);
	}
};
//__PRETTY_FUNCTION__
//__FUNCTION__
#define LOGINOUT LogInOut var##__FUNCTION__((const void *)this, __FUNCTION__);
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
				ui8 * object;
				ui32 count;
				ui32 countWeak;

				DestructionWorker destructor;

				RefCounter():object(0), count(0),countWeak(0), destructor(DestructionWorkerFunc_noOp){LOGINOUT;};
								
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

				ui8 * getObjectLocation(){
					return object;
				}
				const ui8 * getObjectLocation() const{
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
			protected:
				void * p;
				RefCounter * c;
				PtrOffset i;

				Reference():c(&RefCounter::nullCounter), i(0), p(0){}

				Reference(const Reference & other):c(other.c), i(other.i), p(other.p){}

				const Reference & operator = (const Reference & other){
					c = other.c;
					i = other.i;
					p = other.p;
					return *this;
				}

				void operator()(ui32 i = 0){
					p = c->getObjectLocation() + i;
				}

				bool operator == (const Reference & other){
					return (p == other.p);
				}
			};


			template<void (RefCounter::*up)(), void (RefCounter::*down)()>
			class RefHolderBase: public Reference{
				void swap(const Reference & other){LOGINOUT;}
			public:
				RefHolderBase(){LOGINOUT;}
				template<void (RefCounter::*up0)(), void (RefCounter::*down0)()>
				RefHolderBase(const RefHolderBase<up0, down0> & other):Reference(other){LOGINOUT;}
				virtual ~RefHolderBase(){LOGINOUT;}
			};

			template<typename type>
			class Type{
			public:
				Type(){LOGINOUT;}

				template<typename type0> Type(const Type<type0> other){
					LOGINOUT;
					type * newp = (type0 *)0;
				}

			};

			template<typename type, void (RefCounter::*up)(), void (RefCounter::*down)()>
			class RefHolder{
			public:
				RefHolderBase<up, down> m_ref;
				Type<type> m_type;

			public:
				RefHolder(){LOGINOUT;}

				template <typename type2, void (RefCounter::*up2)(), void (RefCounter::*down2)()>
				RefHolder(const RefHolder<type2, up2, down2> other):m_type(other.m_type), m_ref(other.m_ref){LOGINOUT;}
			};

			/*
			template<typename type>
			class WeakPointer: public RefHolder<type, &RefCounter::incrementWeak, &RefCounter::decrementWeak>{
			public:
				WeakPointer(){LOGINOUT}
				WeakPointer(const WeakPointer<type> &other):RefHolder<type, &RefCounter::incrementWeak, &RefCounter::decrementWeak>(other){LOGINOUT;}

				template <type>
			};

			template<typename type>
			class Pointer: public RefHolder<type, &RefCounter::increment, &RefCounter::decrement>{
			public:
				Pointer(){LOGINOUT;}
				Pointer(const Pointer<type> &other):RefHolder<type, &RefCounter::increment, &RefCounter::decrement>(other){LOGINOUT;}
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

