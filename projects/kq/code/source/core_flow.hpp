#ifndef KQ_FLOWS
#define KQ_FLOWS

#include "core_IntegerTypes.hpp"
#include "core_memory_RefCounter.hpp"

namespace kq{
	namespace core{
		namespace flow{

			enum FlowError{
				kErrNone = 0,
				kErrUndefinedProcessorType,
				kErrNotImplemented,
				kErrOutOfMemory,
			};

			struct ProcessorType{
				typedef kq::core::ui32 Length;
				typedef const unsigned char * Localtion;
				Localtion location;
				Length length;
			};

			typedef kq::core::ui32 PortNumber;
			typedef kq::core::ui32 BufferLength;


			class IProcessor{
			public:
				struct Message{

				};

				virtual FlowError perform(Message *) = 0;
			};

			class IConnection{
			public:
				virtual FlowError recommendBufferLength(BufferLength len) = 0;
			};

			class ISocket{
			public:
				virtual FlowError createConnection(PortNumber, ISocket * pSocket, PortNumber);
			};

			class IAssemblyLine{
			public:
				virtual FlowError createProcessor(IProcessor **);
			};

			class IFactory{
			public:
				virtual FlowError createAssemblyLine(ProcessorType *, IAssemblyLine **);
			};

			class IFlowSessionClient{
			public:
				virtual FlowError getLocalFactory(kq::core::memory::Pointer<IFactory> &);

			};

			class IFlowSessionServer{
			public:
				virtual FlowError createSocket(ProcessorType *, kq::core::memory::Pointer<ISocket> &) = 0;
			};

			class IFlowSession{
			public:
				struct FlowsSessionInitOptions{
					kq::core::memory::MemoryWorker * mem;
					kq::core::memory::Pointer<IFlowSessionClient> pClient;

				};
				virtual kq::core::memory::WeakPointer<IFlowSessionServer> getServer() = 0;
				virtual kq::core::memory::WeakPointer<IFlowSessionClient> getClient() = 0;
				static FlowError createFlowSession(FlowsSessionInitOptions *, kq::core::memory::Pointer<IFlowSession> &);
			};
		}
	}
}

#endif
