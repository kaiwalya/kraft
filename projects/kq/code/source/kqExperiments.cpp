#include "core.hpp"

namespace kq{

	namespace flows{
		
		
		class IResource{
		public:
			//The locks can be used as follows
			virtual bool lockResource(bool bExclusive) = 0;
			virtual bool unlockResource() = 0;
		};
		
		class IFlowWriter: public IResource{
			//When a writer is exclusively locked by a processor it means that only that processor can write to this flow.
			//When a writer is shared locked by a processor multiple processors can atomically write to the flow
		public:
			virtual void write(const void * pBytes, kq::core::ui64 nBytes);
		};
		
		class IFlowReader: public IResource{
			virtual void read(void * pBytes, kq::core::ui64 nBytes);
		};

		class IFlowTuple: public IResource{
			//An ordered set of flows
		};

		class IFlowWriterTuple{
		public:
			virtual kq::core::memory::Pointer<IFlowWriter> getFlowWriter(kq::core::ui64 iFlow) = 0;
		};

		class IFlowReaderTuple{
			virtual kq::core::memory::Pointer<IFlowReader> getFlowReader(kq::core::ui64 iFlow) = 0;
			
			virtual void readRecord(void ** pBytes, kq::core::ui64 * nBytes) = 0;
		};
		
		
		class IFlowInterfaceDescriptor{
			
		public:
			virtual void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs, kq::core::ui64 * pOutputs) = 0;
			//e.g. 2, 1
		};

		class IProcessor: public IFlowInterfaceDescriptor{
			//Reads from input streams...processes....writes to output streams
			//Lifecycle: construct...(attach, work, detach)^n....desctruct
		public:
			
			//Sets the output and input streams, acquire locks if required, if not locked queues might be shared.
			virtual void attachFlows(kq::core::memory::Pointer<IFlowWriterTuple> inputs, kq::core::memory::Pointer<IFlowReaderTuple> outputs) = 0;

			//There are changes that signals/setjumps/longjumps may switch the OS level thread without telling this object
			//It is recommended to not do thread local stuff (TLS, etc)
			//Once doWork returns..the object may be destroyed or attach may be called
			virtual void doWork() = 0;
			
			//Detaches the input output streams...remove any references held on streams
			virtual void detachFlows() = 0;
		};
		
		class IProcess:public IFlowInterfaceDescriptor{
			//Represents a process and creates IProcessor objects which actually process streams of data
			//e.g. "The process of merging 2 streams into one
			
		public:
			
			virtual kq::core::memory::Pointer<IProcessor> generateProcessor() = 0;
			//Create a IWorker object
		};


		class IFlow : public IResource{
			//Fifo
		public:
			//User has forgotten this flow object and hence has no way of getting back to this one
			//Which means there can be no more readers attaching to this one...so start forgetting read data
			void disownedByUser();
		};




		class Machine;
		class Value{

			Machine * m_pMachine;

			friend class Variable;
			friend class Machine;
			Value(Machine * pMachine){
				m_pMachine = pMachine;
			}


			void confirmConnection (kq::core::memory::Pointer<Value>){
			}
		public:
			//Once this is called this value can no longer be used by the user
			//This means we can start deleting any stored data on the attached flow.
			~Value(){}
		};

		class Variable{

			kq::core::memory::Pointer<Value> m_pValue;
			friend class Machine;
			Variable(kq::core::memory::Pointer<Value> pVal){
				m_pValue = pVal;
			}
		public:
			Variable operator >> (Variable v){
				
				if(m_pValue && v.m_pValue){
					m_pValue->confirmConnection(v.m_pValue);
				}
				else{
					//error
				}

				return v;
			}

			
			Variable(){}
			Variable(const Variable &){}

			~Variable(){}
		};
		
		class Machine{
			kq::core::memory::MemoryWorker mem;
			
		public:

			Machine(kq::core::memory::MemoryWorker & memory):mem(memory){

			}

			Variable variableFromProcessor(kq::core::memory::Pointer<IProcessor> pProcessor){
				return Variable(kq_core_memory_workerRefCountedClassNew(mem, Value, this));
			}

			Variable variableFromProcess(kq::core::memory::Pointer<IProcess> pProcessor){
				return Variable(kq_core_memory_workerRefCountedClassNew(mem, Value, this));
			}

		};
		
		


	}
}

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;


using namespace kq::flows;


class Input: public IProcessor{
public:
	void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs,kq::core::ui64 * pOutputs){
		*pInputs = 0;
		*pOutputs = 1;
	}

	void attachFlows(Pointer<IFlowWriterTuple> tupleW, Pointer<kq::flows::IFlowReaderTuple> tupleR){
	}

	void doWork(void){
	}

	void detachFlows(void){
	}
};


class Output: public IProcessor{
public:
	void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs,kq::core::ui64 * pOutputs){
		*pInputs = 1;
		*pOutputs = 0;
	}

	void attachFlows(Pointer<IFlowWriterTuple> tupleW, Pointer<kq::flows::IFlowReaderTuple> tupleR){
	}

	void doWork(void){
		
	}

	void detachFlows(void){
	}
};

int main(int /*argc*/, char ** /*argv*/){
	
	//Create std allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);


	PooledMemoryAllocator allocPool(memStd);
	MemoryWorker mem;
	allocPool.getMemoryWorker(mem);


	flows::Variable a,b;
	flows::Machine m(mem);

	Pointer<Input> pInput = kq_core_memory_workerRefCountedClassNew(mem, Input);
	Pointer<Output> pOutput = kq_core_memory_workerRefCountedClassNew(mem, Output);
	a = m.variableFromProcessor(pInput.castStatic<IProcessor>());
	b = m.variableFromProcessor(pOutput.castStatic<IProcessor>());

	a >> b;
	
	return 0;
}