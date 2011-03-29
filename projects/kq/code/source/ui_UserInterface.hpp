#if !defined(KQ_UI_USERINTERFACE_HPP)
#define KQ_UI_USERINTERFACE_HPP


#include "core_memory.hpp"
#include "core_IntegerTypes.hpp"

namespace kq{
    namespace ui{


    	class Screen;
    	class Window;
    	class UserInterface;
    	class OpenGLBridge;

    	class FormatSpecification{
    	public:
    		enum PixelColor{
				pixclRGB_888,
				pixclRGBA_8888,
			};

    		enum RenderDestFlags{
    			//Default is window
    			rfWindow = 0,

    			rfNoWindow = 1,
    			rfMemory = 2,
    		};

    		enum RequestType{

    			rtEnd = 0,
    			rtPixelColorE,
    			rtMinimumDepthBitsU32,
    			rtMinimumStencilBitsU32,
    			rtDoubleBufferingB,
    			rtRenderDestF,
    			rtOpenGLRenderable,
    			rtNativeRenderable,
    		};


    		RequestType requestType;
    		kq::core::ui32 requestValue;
    	};

    	class OpenGLBridge{
    	public:
    	};

    	class Window{
    	public:
    		//vpod getFormatSpecificationValue();
    	};

        //Starting point for any UI related stuff for a machine.
        class UserInterface{
        public:
    		static kq::core::memory::Pointer<UserInterface> createInstance(kq::core::memory::MemoryWorker &mem);

        public:
            virtual kq::core::ui32 getScreenCount() = 0;
            virtual kq::core::memory::Pointer<Screen> getScreen(kq::core::ui32 iScreen) = 0;
            virtual kq::core::memory::Pointer<OpenGLBridge> getOpenGLBridge() = 0;
        };


        class Screen{
        public:
        	virtual kq::core::memory::Pointer<Window> createRootWindow(const FormatSpecification * pRequests) = 0;
        	virtual kq::core::memory::Pointer<UserInterface> getOwner() = 0;
        };


    }
}


#endif
