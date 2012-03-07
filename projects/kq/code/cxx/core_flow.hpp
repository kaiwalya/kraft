#ifndef KQ_FLOWS
#define KQ_FLOWS

#include "core_IntegerTypes.hpp"
#include "core_memory_RefCounter.hpp"

#define HAS_FLOWTRONICS
#define HAS_FLOWSV1

namespace kq{
	namespace core{
		namespace flows3{
			typedef int ProcessorID;
			typedef int PortID;
			typedef int LinkID;


			enum Error{
				kErrNone,
				kErrOutOfMemory,
				kErrUndefinedID,
			};
			typedef Error (*ProcessorFunc)();

			struct FlowsAPI{
				Error (*processor_create)(ProcessorID *, ProcessorFunc);
				Error (*processor_wait)(ProcessorID id);
				Error (*processor_link)(LinkID *, ProcessorID, PortID, ProcessorID, PortID);
				Error (*processor_unlink)(LinkID);
			};

			Error initialize(FlowsAPI *);
			Error finalize(FlowsAPI *);

		}

		namespace flows2{
			enum FlowsError{
				kErrNone,
			};
			typedef void * FlowsConnection;
			FlowsError flows_connect_local(FlowsConnection *);
		}
#if defined(HAS_FLOWTRONICS)
		namespace flowtronics{
			
			enum FlowtronicsError{
				kError_AllOk,
				kError_NotEnoughMemory,
				kError_NotImplemented,
			};

			struct FlowtronicsAPI;
			struct ProcessorServerAPI;
			struct ProcessorClientAPI;

			//struct ProcessorTypeServerAPI;
			struct ProcessorTypeClientAPI;

			struct Name{
				const kq::core::ui8 * location;
				const kq::core::ui32 size;
			};
			
			typedef kq::core::ui32 Port;
			
			struct CreateProcessorContext{
				Name factoryName;
				Name processorTypeName;
				Name ProcessorName;
				
				ProcessorClientAPI * processorClientAPI;
				void * processorClient;
				
				ProcessorServerAPI * processorServerAPI;
				void * processorServer;
			};
			
			struct FactoryRegistration{
				enum Format{
					kFactoryRegistration_Format_LocalFactory,
					kFactoryRegitration_Format_SharedLibrary,
				};
				
				Name factoryName;
				
				struct LocalFactory{
					FlowtronicsError (*(*findFactory)(Name factoryName))(CreateProcessorContext *);
				};
				
				struct SharedLibraryFactory{
					
					enum Arch{
						kFactoryRegistration_SharedLibraryFactory_Arch_unknown,
					};
					
					enum Arch arch;
					const char * filename;
					const char * findFactoryFunctionName;
					
				};
				
				union{
					LocalFactory localFactory;
					SharedLibraryFactory soFactory;
				};
			};
			
			
			struct ProcessorServerAPI{
				FlowtronicsError (*requestHalt)();
				FlowtronicsError (*requestRegisterFactory)(void * processor, FactoryRegistration *);
				FlowtronicsError (*requestCreateProcessor)(void * processor, CreateProcessorContext * c);
				FlowtronicsError (*requestLink)(void * processor, Port thisport, void * processorOther, Port otherport);
				FlowtronicsError (*requestReadFrom)(Port p);
				FlowtronicsError (*requestWriteTo)(Port p);
			};
			
			struct ProcessorClientAPI{
				//The first client API call to be called
				FlowtronicsError (*onPowerUp)(void * obj);
				
				//Called when ever an exception occurs
				FlowtronicsError (*onException)(void * obj);
				
				//Called when someone dispatched a read/write requiest
				FlowtronicsError (*onDataRequest)(void * obj);
				
				//Last Client api to be called
				FlowtronicsError (*onPowerDown)(void * obj);
				
			};
			
		
			struct FlowtronicsAPI{
				FlowtronicsError (*createRootProcessor)(CreateProcessorContext * c);
			};
			
			bool getFlowtronicsAPI(FlowtronicsAPI *);
			
		};
#endif
#if defined (HAS_FLOWSV1)		
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
#endif
	}
}

#endif
