#ifndef WORKER_H_
#define WORKER_H_

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

	void set(ContextType pContext, FunctionType pfnWork){
		context = pContext;
		workfn = pfnWork;
	}

};

class MemoryWorker:public Worker<void *, void *(*)(void *, void *, ui64)>{
public:
	void * operator()(void * p, ui64 n){
		return (*getWorkerFunction())(getWorkerContext(), p, n);
	};
};


#endif