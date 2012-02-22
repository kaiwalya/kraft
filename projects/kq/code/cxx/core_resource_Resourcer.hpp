#include "core_IntegerTypes.hpp"

namespace kq{
	namespace core{
		namespace resource{
			class Resourcer{
			public:
				typedef kq::core::ui32 ResourceCount;

				class ResourceManager:virtual kq::core::debug::Log{
					volatile static kq::core::ui32 nResourcers;
				public:
					ResourceManager(){enableLogging();}
					~ResourceManager(){
						log("%d resourcers leaked\n", nResourcers);
					}
					void operator+=(ResourceCount c){
						nResourcers+=c;
					}
					void operator-=(ResourceCount c){
						nResourcers-=c;
					}
				};

				static ResourceManager manager;
				kq::core::ui32 nResources;
			public:
				Resourcer():nResources(0){}
				~Resourcer(){kq::core::debug::assume(!nResources);}
				void initialize(int iCount = 1){manager+=iCount;nResources+=iCount;}
				void finalize(int iCount = 1){manager-=iCount;nResources-=iCount;}
			};
		}
	}
}
