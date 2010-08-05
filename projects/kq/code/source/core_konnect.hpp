#ifndef KQ_FLOWS_KONNECT
#define KQ_FLOWS_KONNECT

#include "core_IntegerTypes.hpp"
#include "core_memory_RefCounter.hpp"

namespace kq{
	namespace core{
		namespace konnect{
			
			template<typename NodeID>
			class IGraph{			
			
				
				class IModifier{
					enum Error{
						errNone = 0,
						errUnknown,
						errBadAPIUsage,
					};

					enum Mode{
						modeChain,
						modePair,
						modeOneToMany,
						modeManyToOne,
						modeClique,
					};

					switchMode(Mode m);
					virtual Error node(NodeID id) = 0;
					virtual Error nodeVector(NodeID * pNodes, kq::core::ui32 nNodes) = 0;
					virtual Error nodeString(NodeID * pNodes) = 0;
				};

				enum ModifierType{
					modConnect,
					modDisconnect,
				};

				class IViewer{
				};

				
				virtual kq::core::memory::Pointer<IMutator> getMutator(ModifierType type) = 0			
				virtual kq::core::memory::Pointer<IReader> getViewer() = 0
				

			};

		};
	};
};

#endif