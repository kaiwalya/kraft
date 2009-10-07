#ifndef SYSTEM_XWINDOWS_H_
#define SYSTEM_XWINDOWS_H_


#include "../IWindowManager.hpp"

#include "X11/Xlib.h"
#include "GL/glx.h"

namespace dos{
	namespace system{
		namespace X{
			class XWindows : public dos::system::IWindowManager{
				Display * m_pXDisplay;
				XVisualInfo * m_pXVisualInfo;
				Colormap m_colormap;
				Window m_hWindow;
				GLXContext m_hContext;
				Atom m_atomWindowCloseEvent;
			public:
				void up(void **);
				void down(void **);
				void process(void **);
				void updateFrame(void **);
			};
		}
	}
}

#endif
