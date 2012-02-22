#ifndef WORKER_H_
#define WORKER_H_


namespace kq{

	namespace core{

		template<typename ContextType, typename FunctionType>
		class Worker{
		private:
			ContextType context;
			//void * (*workfn)(void * context, void * p, ui64 n);
			FunctionType workfn;
		protected:
			FunctionType getWorkerFunction()const{
				return workfn;
			}

			ContextType getWorkerContext()const{
				return context;
			}

		public:
			
			Worker(){
				set(0,0);
			};

			Worker(const Worker & worker){
				set(worker.getWorkerContext(), worker.getWorkerFunction());
			}

			void set(ContextType pContext, FunctionType pfnWork){
				context = pContext;
				workfn = pfnWork;
			}

		};


	};

}


#endif
