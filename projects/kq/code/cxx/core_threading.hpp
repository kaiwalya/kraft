#if !defined(KQ_CORE_THREADING_H_)
#define KQ_CORE_THREADING_H_

namespace kq{
	namespace core{
		namespace threading{
			typedef int ThreadID;

			enum Error{
				kErrNone,
				kErrNotImplemented,
				kErrOutOfMemory,
				kErrBadState,
			};

			typedef Error (*ThreadFunc)(void *);

			class IThread{
			public:
				static Error constuctThread(IThread **, ThreadFunc, void * data);

				//Start execution
				virtual Error start() = 0;

				//Wait for exit
				virtual Error wait(Error * out) = 0;

				//Kill thread
				virtual Error kill() = 0;

				virtual ~IThread();

			};

			class IMutex{
			public:
				static Error constructMutex(IMutex **);
				virtual Error lock() = 0;
				virtual Error unlock() = 0;
				virtual ~IMutex();
			};

			class ICondition: public IMutex{
			public:
				static Error constructCondition(ICondition **);
				virtual Error signal() = 0;
				virtual Error wait() = 0;
				virtual ~ICondition();
			};

			class ScopeLock{
				IMutex * m;
			public:
				ScopeLock(IMutex * m);
				~ScopeLock();
			};

			/*
			class IContext{
			public:
				static Error constructCallContext(IContext **, ThreadFunc func, void * data);
				static Error constructEmptyContext(IContext **);
				virtual Error load(IContext * saveOld);
				virtual Error save();
				virtual ~IContext();
			};
			*/
		}
	}
}

#endif

