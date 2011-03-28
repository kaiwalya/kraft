#include "ui_X_UserInterface.hpp"

//#include "GL/gl.h"
#include "GL/glx.h"
#include "X11/Xlib.h"

#include "stdio.h"
#include "memory.h"

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;

#include "core_data_BPlusTree.hpp"


namespace kq{
    namespace ui{
        namespace X{

        	class UserInterface;
        	class XScreen;

            class UserInterface: public RefCounted, public ui::UserInterface{
            protected:
            	MemoryWorker mem;
            protected:
                Display * m_pDisplay;
            public:
                Display * getDisplay();


            public:
                ui32 getScreenCount();
                Pointer<ui::Screen> getScreen(ui32);



            protected:
                friend Pointer<ui::UserInterface> ui::X::createInstance(MemoryWorker &mem);
                UserInterface(RefCounter * pCounter, MemoryWorker &memworker);
                bool up();
                void down();
            public:
                ~UserInterface();
            };

            class XScreen:public RefCounted{
            	MemoryWorker mem;
            	Pointer<UserInterface> m_pUserInterface;
            	ui32 m_iIndex;
            protected:
            	void loadConfigs();
            public:
            	XScreen(RefCounter * pCounter, MemoryWorker mem);

            	bool up(UserInterface * pUserInterface, ui32 iIndex);
            	void down();

            	~XScreen();
            };

        }
    }
}


Pointer<ui::UserInterface> ui::X::createInstance(MemoryWorker &mem){
    Pointer<UserInterface> pRet;

    kq_core_memory_workerRefCountedObjectNew(pRet, mem, UserInterface, mem);
    if(pRet->up()){
    	return pRet.castStatic<ui::UserInterface>();
    }
    return 0;
}


using namespace kq::ui::X;


XScreen::XScreen(RefCounter * pRef, MemoryWorker memworker)
	:RefCounted(pRef),
	 mem(memworker)
{

}

XScreen::~XScreen(){}

bool XScreen::up(UserInterface * pUserInterface, ui32 iIndex){
	m_pUserInterface = pUserInterface->ref<Pointer<UserInterface> >();
	if(m_pUserInterface){
		m_iIndex = iIndex;
		loadConfigs();
		return true;
	}
	return false;
}

struct GLXAtrribute{
	int iAttr;
	const char * pDescription;
};

static const GLXAtrribute attribs[] = {
		{GLX_FBCONFIG_ID, "id"},
		{GLX_BUFFER_SIZE, "bufsz"},
		{GLX_LEVEL, "lvl"},
		{GLX_DOUBLEBUFFER, "dblbuff"},
		{GLX_STEREO, "str"},
		{GLX_AUX_BUFFERS, "auxbuf"},
		{GLX_RED_SIZE, "r"},
		{GLX_GREEN_SIZE, "g"},
		{GLX_BLUE_SIZE, "b"},
		{GLX_ALPHA_SIZE, "a"},
		{GLX_DEPTH_SIZE, "depth"},
		{GLX_STENCIL_SIZE, "stencil"},
		{GLX_ACCUM_RED_SIZE, "ar"},
		{GLX_ACCUM_GREEN_SIZE, "ag"},
		{GLX_ACCUM_BLUE_SIZE, "ab"},
		{GLX_ACCUM_ALPHA_SIZE, "aa"},
		{GLX_RENDER_TYPE, "rndrtyp"},
		{GLX_DRAWABLE_TYPE, "drwtyp"},
		{GLX_X_RENDERABLE, "xrend"},
		{GLX_VISUAL_ID, "VID"},
		{GLX_X_VISUAL_TYPE, "VisTyp"},
		{GLX_CONFIG_CAVEAT, "ceveat"},
		{GLX_TRANSPARENT_TYPE, "trType"},
		{GLX_TRANSPARENT_INDEX_VALUE, "trIDX"},
		{GLX_TRANSPARENT_RED_VALUE, "trR"},
		{GLX_TRANSPARENT_GREEN_VALUE, "trG"},
		{GLX_TRANSPARENT_BLUE_VALUE, "trB"},
		{GLX_TRANSPARENT_ALPHA_VALUE, "trA"},
		{GLX_MAX_PBUFFER_WIDTH, "pbufW"},
		{GLX_MAX_PBUFFER_HEIGHT, "pbufH"},
		{GLX_MAX_PBUFFER_PIXELS, "pbufPix"},
};

static const int nAttributes = sizeof(attribs)/sizeof(attribs[0]);

struct ConfigAttributes{
	int val[nAttributes];
};

void XScreen::loadConfigs(){
	int nConfigs = 0;
	GLXFBConfig * pConfigs = glXGetFBConfigs(m_pUserInterface->getDisplay(), m_iIndex, &nConfigs);
	if(pConfigs && nConfigs){
		printf("Screen %d, found %d configs\n", m_iIndex, nConfigs);

		ConfigAttributes * pCA = (ConfigAttributes *)mem(0, nConfigs * sizeof(ConfigAttributes));
		memset(pCA, -1, sizeof(*pCA) * nConfigs);
		int iConfig = 0;
		while(iConfig < nConfigs){
			ConfigAttributes & ca = pCA[iConfig];
			int iAttrib = 0;
			while(iAttrib < nAttributes){
				glXGetFBConfigAttrib(m_pUserInterface->getDisplay(), pConfigs[iConfig], attribs[iAttrib].iAttr, &(ca.val[iAttrib]));
				iAttrib++;
			}


			//Create the tree;
			kq::core::data::BPlusTree mapIDToResultOffset(mem, sizeof(int));
			{
				iAttrib = 0;;
				while(iAttrib < nAttributes){
					mapIDToResultOffset.map(&attribs[iAttrib].iAttr, &(ca.val[iAttrib]) );
					iAttrib++;
				}
			}

			int iKey;
			iKey = GLX_VISUAL_ID;
			int iVID = *(int *)mapIDToResultOffset.lookup(&iKey);
			iKey = GLX_CONFIG_CAVEAT;
			int iCAVEAT = *(int *)mapIDToResultOffset.lookup(&iKey);
			iKey = GLX_DOUBLEBUFFER;
			int iDoubleBuffer = *(int *)mapIDToResultOffset.lookup(&iKey);
			iKey = GLX_RED_SIZE;
			int iRBits = *(int *)mapIDToResultOffset.lookup(&iKey);
			iKey = GLX_ALPHA_SIZE;
			int iABits = *(int *)mapIDToResultOffset.lookup(&iKey);

			if(iVID && iCAVEAT == GLX_NONE && iDoubleBuffer && iRBits == 8 && iABits == 8){
				iAttrib = 0;
				while((iAttrib < nAttributes)){
					int key = attribs[iAttrib].iAttr;
					if(
							key != GLX_FBCONFIG_ID &&
							key != GLX_MAX_PBUFFER_PIXELS &&
							key != GLX_LEVEL &&
							key != GLX_VISUAL_ID &&
							key != GLX_BUFFER_SIZE &&
							key != GLX_ACCUM_RED_SIZE &&
							key != GLX_ACCUM_GREEN_SIZE &&
							key != GLX_ACCUM_BLUE_SIZE &&
							key != GLX_ACCUM_ALPHA_SIZE &&
							true
							){
						printf("%s %x, ", attribs[iAttrib].pDescription, ca.val[iAttrib]);
					}
					iAttrib++;
				}
				printf("\n");
			}
			iConfig++;

		}

	}
}

UserInterface::UserInterface(RefCounter * pCounter, MemoryWorker &memworker):
		RefCounted(pCounter),
		mem(memworker),
		m_pDisplay(0)
{

}

bool UserInterface::up(){
	bool bRet = false;
	m_pDisplay = XOpenDisplay(0);
	if(m_pDisplay){
		bRet = true;
	}
}

void UserInterface::down(){
	if(m_pDisplay){
		XCloseDisplay(m_pDisplay);
		m_pDisplay = 0;
	}
}

UserInterface::~UserInterface(){
	down();
}

ui32 UserInterface::getScreenCount(){
	ui32 iRet = (ui32)XScreenCount(m_pDisplay);
	return iRet;
}

Display * UserInterface::getDisplay(){
	return m_pDisplay;
}

Pointer<ui::Screen> UserInterface::getScreen(ui32 iScreen){

	Pointer<XScreen> p;
    kq_core_memory_workerRefCountedObjectNew(p, mem, XScreen, mem);
    if(p){
    	if(p->up(this, iScreen)){
    		return p;
    	}
    }
    return 0;
}

