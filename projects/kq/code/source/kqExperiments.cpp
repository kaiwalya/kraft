#include "core.hpp"

namespace kq{

	namespace flows{
		
		
		class IResource{
		public:
			//The locks can be used as follows
			virtual bool lockResource(bool bExclusive) = 0;
			virtual bool unlockResource() = 0;
		};
		
		
		class IFlow: public IResource{
			//FIFO abstraction
		};
		
		class IFlowWriter: public IResource{
			//When a writer is exclusively locked by a processor it means that only that processor can write to this flow.
			//When a writer is shared locked by a processor 
		public:
			virtual void inject(void * pBytes, kq::core::ui64 nBytes);
		};
		
		class IFlowReader: public IResource{
			virtual void read
		};
		
		
		class IProcessor{
			//Reads from input streams...processes....writes to output streams
			//Lifecycle: construct...(attach, work, detach)^n....desctruct
		public:
			
			//Sets the output and input streams, acquire locks if required, if not locked queues might be shared.
			virtual void attachStreams(kq::core::memory::Pointer<IStreamTupleReader> inputs, kq::core::memory::Pointer<IStreamTupleWriter> outputs) = 0;

			//There are changes that signals/setjumps/longjumps may switch the OS level thread without telling this object
			//It is recommended to not do thread local stuff (TLS, etc)
			//Once doWork returns..the object may be destroyed or attach may be called
			virtual void doWork() = 0;
			
			//Detaches the input output streams...remove any references held on streams
			virtual void detachStreams() = 0;
		};
		
		class IProcess{
			//Represents a process and creates IProcessor objects which actually process streams of data
			//e.g. "The process of merging 2 streams into one
			
		public:
			
			void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs, kq::core::ui64 * pOutputs);
			//e.g. 2, 1
			
			kq::core::memory::Pointer<IWorker> generateWorker();
			//Create a IWorker object
		};
	}
}

int main(int argc, char ** argv){
	
	
	
	return 0;
}