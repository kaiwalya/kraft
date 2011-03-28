#if !defined(KQ_UI_USERINTERFACE_HPP)
#define KQ_UI_USERINTERFACE_HPP


#include "core_memory.hpp"
#include "core_IntegerTypes.hpp"

namespace kq{
    namespace ui{


    	class Screen;
    	class UserInterface;


        //Starting point for any UI related stuff for a machine.
        class UserInterface{
        public:
        public:
    		static kq::core::memory::Pointer<UserInterface> createInstance(kq::core::memory::MemoryWorker &mem);

        public:
            virtual kq::core::ui32 getScreenCount() = 0;
            virtual kq::core::memory::Pointer<Screen> getScreen(kq::core::ui32 iScreen) = 0;
        };


        class Screen{

        };


    }
}


#endif
