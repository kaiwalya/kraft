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
//#define LOGINOUT LogInOut var##__FUNCTION__((const void *)this, __FUNCTION__);
//#define LOGDEPTH var##__FUNCTION__.pushdepth()
#define LOGINOUT ((void)0)
#define LOGDEPTH LOGINOUT


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
					//if(this != &nullCounter)printf("[%8p]++ > %d\n", this, count);
					//LOGDEPTH;printf("[%p]w %d, s %d\n", this, count, countWeak);
				};

				void incrementWeak(){
					LOGINOUT;
					++countWeak;
					//LOGDEPTH;printf("[%p]w %d, s %d\n", this, count, countWeak);
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
					//LOGDEPTH;printf("[%p]w %d, s %d\n", this, count, countWeak);
					//if(this != &nullCounter)printf("[%8p]-- > %d\n", this, count);
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
					//LOGDEPTH;printf("[%p]w %d, s %d\n", this, count, countWeak);
					if(countWeak == 0){
						if(count == 0 && this != &nullCounter){
							destructor(this, 0);
						}
					}
				}

			};

			template<typename type, void (RefCounter::*up)(), void (RefCounter::*down)()>
			class RefHolder{
				template<typename type2, void (RefCounter::*up2)(), void (RefCounter::*down2)()> friend class RefHolder;

				#define kq_declare_template template <typename type0, void (RefCounter::*up0)(), void (RefCounter::*down0)()>
				#define kq_declare_function_taking_constreference(name) kq_declare_template name (const RefHolder<type0, up0, down0> & o)
				#define kq_overload_bool_operator(op) kq_declare_function_taking_constreference(bool operator op) const{return ((p()) op (o.p()));}
			protected:
				type * t;
				RefCounter * c;
				PtrOffset i;

				void switchCounter(RefCounter * oc, PtrOffset oi){
					//printf("Switching counters\n");
					if(c != oc){
						(oc->*up)();
						(c->*down)();
						c = oc;
						i = oi;
					}
				}

				type * p(){
					type * p0;
					if(c != &RefCounter::nullCounter){
						p0 = (type *)c->getObjectLocation();
						if(!p0){
							switchCounter(&RefCounter::nullCounter, i);
						}
					}
					else{
						p0 = 0;
					}
					return (p0 + i);
				}

				RefHolder(RefCounter * oc, PtrOffset oi):c(oc?oc:&RefCounter::nullCounter), i(oi), t(0){(c->*up)();}
			public:
				RefHolder():c(&RefCounter::nullCounter), i(0), t(0){(c->*up)();}
				RefHolder(const RefHolder & o):c(o.c), i(o.i), t(o.t){(c->*up)();}
				~RefHolder(){(c->*down)();}
				kq_declare_template explicit RefHolder(const RefHolder<type0, up0, down0> & o):c(o.c), i(o.i), t(o.t){(c->*up)();}
				kq_declare_function_taking_constreference(ui32 operator -){return ((char *)p() - (char *)p());}
				kq_overload_bool_operator(==);
				kq_overload_bool_operator(!=);
				kq_overload_bool_operator(<);
				kq_overload_bool_operator(<=);
				kq_overload_bool_operator(>=);
				kq_overload_bool_operator(>);
				RefHolder & operator +=(ui32 iN){i += iN;}
				RefHolder & operator -=(ui32 iN){i -= iN;}
				RefHolder & operator ++(){i++;}
				RefHolder & operator --(){i--;}
				void operator ++(int){i++;}
				void operator --(int){i--;}
				type & operator [](ui32 iOffset){return *(p() + iOffset);}
				type * operator -> (){return p();}
				type & operator * (){return *(p());}
				operator bool(){return (p()!=0);}
				RefHolder<type,up,down> & operator =(const RefHolder<type,up,down> & o){
					switchCounter(o.c, o.i);
					t = o.t;
					return *this;
				}


				#undef kq_overload_bool_operator
				#undef kq_declare_function_taking_constreference
				#undef kq_declare_template

			};


			template<typename type>
			class WeakPointer: public RefHolder<type, &RefCounter::incrementWeak, &RefCounter::decrementWeak>{
			public:
				WeakPointer(){}
				WeakPointer(RefCounter * oc, PtrOffset oi = 0):RefHolder<type, &RefCounter::incrementWeak, &RefCounter::decrementWeak>(oc, oi){}
				template<typename type2, void (RefCounter::*up2)(), void (RefCounter::*down2)()>
				WeakPointer(const RefHolder<type2, up2, down2> &other):RefHolder<type, &RefCounter::incrementWeak, &RefCounter::decrementWeak>(other){}

			};

			template<typename type>
			class Pointer: public RefHolder<type, &RefCounter::increment, &RefCounter::decrement>{
			public:
				Pointer(){}
				Pointer(RefCounter * oc, PtrOffset oi = 0):RefHolder<type, &RefCounter::increment, &RefCounter::decrement>(oc, oi){}
				template<typename type2, void (RefCounter::*up2)(), void (RefCounter::*down2)()>
				Pointer(const RefHolder<type2,up2,down2> &other):RefHolder<type, &RefCounter::increment, &RefCounter::decrement>(other){}
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

