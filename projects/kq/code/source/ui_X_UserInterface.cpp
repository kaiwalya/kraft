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

        	class XFreeable{
        		void * m_pXFreeable;
        	public:
        		XFreeable(void * pXFreeable):m_pXFreeable(pXFreeable){}
        		virtual ~XFreeable(){XFree(m_pXFreeable);}
        	};

        	class UserInterface;
        	class XScreen;
        	class XWindow;
        	class GLX;

            class UserInterface: public RefCounted, public ui::UserInterface{
            protected:
            	MemoryWorker & mem;
            public:
            	MemoryWorker & getMemoryWorker(){return mem;}
            protected:
                Display * m_pDisplay;
            public:
                Display * getDisplay();


            public:
                ui32 getScreenCount();
                Pointer<ui::Screen> getScreen(ui32);
                Pointer<ui::OpenGLBridge> getOpenGLBridge();



            protected:
                friend Pointer<ui::UserInterface> ui::X::createInstance(MemoryWorker &mem);
                UserInterface(RefCounter * pCounter, MemoryWorker & mem);
                bool up();
                void down();
            public:
                ~UserInterface();
            };


            class XScreen:public RefCounted, public ui::Screen{

            private:
            	//MemoryWorker & mem;
            	Pointer<UserInterface> m_pUserInterface;
            	ui32 m_iIndex;
            public:
            	XScreen(RefCounter * pCounter);
            	bool up(Pointer<UserInterface> pUserInterface, ui32 iIndex);
            	void down();
            	~XScreen();
            	ui32 getScreenIndex(){
            		return m_iIndex;
            	}
            //Interface
			public:
            	//virtual Pointer<ui::Window> createWindow(const ui::FormatSpecification * pRequests);
            	virtual kq::core::memory::Pointer<ui::Window> createRootWindow(const FormatSpecification * pRequests);
            	virtual Pointer<ui::UserInterface> getOwner();

            };

            class XWindow: public RefCounted, public ui::Window{

            public:
            	class Configuration:public XFreeable{
            	public:
            		XVisualInfo * m_pXVisualInfo;

            	public:
            		Configuration(XVisualInfo * pInfo):XFreeable(pInfo), m_pXVisualInfo(pInfo){

            		}
            	};

            	class Configurator{

            	public:
            		struct FBAEntry{
						int XID;
						int val;
					};
					enum FBAID{
						fbaConfigID = 0,	fbaBufferSize,	fbaOverlayLevel,	fbaDoubleBuffer,
						fbaStereo,			fbaAuxBuffers,	fbaRSize,			fbaGSize,
						fbaBSize,			fbaASize,		fbaDepthSize,		fbaStencilSize,
						fbaARSize,			fbaAGSize,		fbaABSize,			fbaAASize,
						fbaRenderType,		fbaDrawType,	fbaXRenderable,		fbaVisualType,
						fbaCfgCaveat,		fbaTransType,	fbaTransI,			fbaTransR,
						fbaTransG,			fbaTransB,		fbaTransA,			fbaVisualID,
						fbaMaxPBuffW,	fbaMaxPBufH,		fbaMaxPBufPix,
					};

					static const ui32 nRequestAttributes = fbaVisualID;
					static const ui32 nQueryAttributes = (fbaMaxPBufH + 1);

					class Request{

					public:
						FBAEntry list[nRequestAttributes + 1];
						Request();
						int & operator[](FBAID id){
							if(id < nRequestAttributes){
								return list[id].val;
							}
							return list[nRequestAttributes].val;
						}

						//bool isDefault(FBAID id) const;
					};

            	};

            private:
            	::Window m_w;
            	Pointer<XScreen> m_pScreen;
            	bool m_bOwned;
            	XWindow(RefCounter *);
            	bool up(Pointer<XScreen>, ::Window w, bool bOwned = false);
            	bool up(Pointer<XScreen> pScreen, Pointer<XWindow> pParent, Pointer<XWindow::Configuration> pConfig);
            	void down();
            	::Window getWindow(){return m_w;}
            public:

            	~XWindow();

            public:
            	static Pointer<XWindow> createRootWindow(Pointer<XScreen> pScreen, const ui::FormatSpecification * pRequests);
            };

            class GLX:public RefCounted, public ui::OpenGLBridge{

            public:
            	class Configuration{
            		Pointer<XWindow::Configuration> m_pXWindowConfig;
            		GLXFBConfig m_conf;
            		Pointer<UserInterface> m_pUserInterface;
            	public:
            		Configuration(Pointer<UserInterface> pUserInterface, GLXFBConfig conf):m_conf(conf), m_pUserInterface(pUserInterface){}
            		Pointer<XWindow::Configuration> getXWindowConfiguration(){
            			if(!m_pXWindowConfig){
            				XVisualInfo * pInfo = glXGetVisualFromFBConfig(m_pUserInterface->getDisplay(), m_conf);
            				if(pInfo){
            					m_pXWindowConfig = kq_core_memory_workerRefCountedClassNew(m_pUserInterface->getMemoryWorker(), XWindow::Configuration, pInfo);
            				}
            			}
            			return m_pXWindowConfig;
            		}
            	};

            private:
            	Pointer<UserInterface> m_pUserInterface;
            	Display * m_pDisplay;
            	i32 m_iErrorBase;
            	i32 m_iEventBase;
            	i32 m_iVersionMajor;
            	i32 m_iVersionMinor;
            	const char * m_pClientString[3];

            public:
            	GLX(RefCounter * pCounter);~GLX();
            	bool up(Pointer<UserInterface>);
            	void down();
            	Pointer<Configuration> findConfiguration(Pointer<XScreen>, const XWindow::Configurator::Request & r);
            	static Pointer<GLX> createGLX(MemoryWorker &mem, Pointer<UserInterface>);

            };


        }
    }
}


Pointer<ui::UserInterface> ui::X::createInstance(MemoryWorker &mem){
    Pointer<UserInterface> pRet;

    kq_core_memory_workerRefCountedObjectNew(pRet, mem, UserInterface, (pCounter, mem) );
    if(pRet->up()){
    	return pRet.castStatic<ui::UserInterface>();
    }
    return 0;
}


using namespace kq::ui::X;

UserInterface::UserInterface(RefCounter * pCounter, MemoryWorker & memworker):
		RefCounted(pCounter),
		mem(memworker)
{

}

UserInterface::~UserInterface(){
	down();
}

bool UserInterface::up(){
	bool bRet = false;
	m_pDisplay = XOpenDisplay(0);
	if(m_pDisplay){
		printf("X [Display %p] Initialized\n", m_pDisplay);
		bRet = true;
	}
}

void UserInterface::down(){
	if(m_pDisplay){
		printf("X [Display %p] Finalized\n", m_pDisplay);
		XCloseDisplay(m_pDisplay);
		m_pDisplay = 0;
	}
}

Display * UserInterface::getDisplay(){
	return m_pDisplay;
}

ui32 UserInterface::getScreenCount(){
	ui32 iRet = (ui32)XScreenCount(m_pDisplay);
	return iRet;
}

Pointer<ui::Screen> UserInterface::getScreen(ui32 iScreen){

	Pointer<XScreen> p;
    kq_core_memory_workerRefCountedObjectNew(p, mem, XScreen, (pCounter) );
    if(p){
    	if(p->up(This, iScreen)){
    		return p.castStatic<ui::Screen>();
    	}
    }
    return 0;
}

Pointer<ui::OpenGLBridge> UserInterface::getOpenGLBridge(){
	return GLX::createGLX(mem, This).castStatic<ui::OpenGLBridge>();
}


Pointer<XWindow> XWindow::createRootWindow(Pointer<XScreen> pScreen, const ui::FormatSpecification * pRequests){
	typedef FormatSpecification FS;

	Pointer<ui::Window> pRet;
	bool bUseOpenGLBridge = false;

	ui32 nR = 0;
	while(pRequests[nR++].requestType != FS::rtEnd);

	Configurator::Request ar;

	ui32 iR = 0;
	printf("XWindow create [");
	while(iR < nR){

		const FS &r = pRequests[iR];
		const FS::RequestType &k = r.requestType;
		const ui32 &v = r.requestValue;

		switch(k){
		case FS::rtEnd:
			break;
		case FS::rtPixelColorE:
		{
			switch(r.requestValue){
			case FS::pixclRGBA_8888:
				ar[Configurator::fbaASize] = 1;
				printf("Alpha 8, ");
			case FS::pixclRGB_888:
				ar[Configurator::fbaRSize] = 1;
				ar[Configurator::fbaGSize] = 1;
				ar[Configurator::fbaBSize] = 1;
				ar[Configurator::fbaRenderType] = GLX_RGBA_BIT;
				printf("RGB 888, RType RGBA_BIT,");
			}
			break;
		}
		case FS::rtMinimumDepthBitsU32:
			ar[Configurator::fbaDepthSize] = v;
			printf("Depth %d,", v);
			break;
		case FS::rtMinimumStencilBitsU32:
			ar[Configurator::fbaStencilSize] = v;
			printf("Stencil %d,", v);
			break;
		case FS::rtDoubleBufferingB:
			ar[Configurator::fbaDoubleBuffer] = True;
			printf("DoubleBuffer %d,", v);
			break;
		case FS::rtRenderDestF:
			if(v){
				int & out = ar[Configurator::fbaDrawType];
				if(v & FS::rfNoWindow){
					printf("NoWindow, ");
					out =  out & ~(GLX_WINDOW_BIT);

				}
				if(v & FS::rfMemory){
					printf("Memory, ");
					out = out | (GLX_PBUFFER_BIT);
				}
			}
			break;
		case FS::rtOpenGLRenderable:
		{
			printf("OpenGL, ");
			bUseOpenGLBridge = v ? true:false;
			break;
		}
		case FS::rtNativeRenderable:
			printf("XRenderable, ");
			ar[Configurator::fbaXRenderable] = True;
			break;
		default:
			break;
		}
		iR++;
	}
	printf("]\n");
	Pointer<GLX::Configuration> pConfig;
	if(bUseOpenGLBridge){

		Pointer<GLX> pGLX = pScreen->getOwner()->getOpenGLBridge();
		if(pGLX){
			//Check if GLX is supported
			pConfig = pGLX->findConfiguration(pScreen, ar);
		}

		if(pConfig){
			Pointer<Configuration> pXConfig = pConfig->getXWindowConfiguration();
			if(pXConfig){
				Pointer<UserInterface> pUI = pScreen->getOwner();
				Display * pDisplay = pUI->getDisplay();
				Pointer<XWindow> pRoot;
				MemoryWorker & mem = pUI->getMemoryWorker();
				kq_core_memory_workerRefCountedObjectNew(pRoot, mem, XWindow, (pCounter));
				if(pRoot && pRoot->up(pScreen, XDefaultRootWindow(pDisplay))){
					Pointer<XWindow> pNew;
					kq_core_memory_workerRefCountedObjectNew(pNew, mem, XWindow, (pCounter));
					if(pNew && pNew->up(pScreen, pRoot, pXConfig)){
						pRet = pNew;
					}
				}
			}
		}
	}
	else{
		//Use XWindow to find config
	}


	//pScreen->createWindow(pScreen->getOwner().castStatic<UserInterface>()->getDisplay(), XRootWindowOfScreen());
	return 	pRet;

}



XWindow::XWindow(RefCounter * pCounter)
	:RefCounted(pCounter)
{
	printf("Window %p Created\n", this);
}

bool XWindow::up(Pointer<XScreen> pScreen, ::Window w, bool bOwned){
	m_pScreen = pScreen;
	m_w = w;
	m_bOwned = bOwned;
	return true;
}

bool XWindow::up(Pointer<XScreen> pScreen, Pointer<XWindow> pParent, Pointer<Configuration> pConfig){
	Pointer<UserInterface> pUserInterface = pScreen->getOwner();
	MemoryWorker mem = pUserInterface->getMemoryWorker();
	Display * pDisplay = pUserInterface->getDisplay();
	XVisualInfo * pInfo = pConfig->m_pXVisualInfo;

	XSetWindowAttributes swa;

	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask;
	swa.colormap = XCreateColormap(pDisplay, pParent->getWindow(), pInfo->visual, AllocNone);
	int swamask = CWColormap | CWBorderPixel | CWEventMask;
	::Window w = XCreateWindow(pDisplay, pParent->getWindow(), 0, 0, 100, 100, 0, pInfo->depth, InputOutput, pInfo->visual, swamask, &swa);

	XFreeColormap(pDisplay, swa.colormap);

	if(w){
		return up(pScreen, w, true);
	}

	return false;
}

void XWindow::down(){

}

XWindow::~XWindow(){

	down();

	m_pScreen = 0;
	printf("Window %p Destroyed\n", this);

}


Pointer<GLX> GLX::createGLX(MemoryWorker &mem, Pointer<UserInterface> pUserInterface){
	 Pointer<GLX> pRet;

	kq_core_memory_workerRefCountedObjectNew(pRet, mem, GLX, (pCounter));
	if(pRet->up(pUserInterface)){
		return pRet;
	}
	return 0;
}

GLX::GLX(RefCounter * pRef)
	:RefCounted(pRef)
{

}


bool GLX::up(Pointer<UserInterface> pUserInterface){
	m_pUserInterface = pUserInterface;
	if(m_pUserInterface){
		m_pDisplay = m_pUserInterface->getDisplay();
		if(m_pDisplay){
			if(True == glXQueryExtension(m_pDisplay, &m_iErrorBase, &m_iEventBase)){
				printf("glX %p [ErrorBase %d][EventBase %d]", this, m_iErrorBase, m_iEventBase);
				if(True == glXQueryVersion(m_pDisplay, &m_iVersionMajor, &m_iVersionMinor)){
					printf("[Version %d.%d]", m_iVersionMajor, m_iVersionMinor);
					if(m_iVersionMajor >= 1 && m_iVersionMinor >= 4){
						ui32 keys[] = {GLX_VENDOR, GLX_VERSION, GLX_EXTENSIONS};
						const char * keynames[] = {"GLX_VENDOR", "GLX_VERSION", "GLX_EXTENSIONS" };
						ui32 iKey, nKeys;
						nKeys = sizeof(keys)/sizeof(keys[0]);
						for(iKey = 0; iKey < nKeys; iKey++){
							m_pClientString[iKey] = glXGetClientString(m_pDisplay, keys[iKey]);
							if(!iKey)printf("[%s \"%s\"]", keynames[iKey], m_pClientString[iKey]);
						}

						printf(".....glX Initialized\n");
						return true;
					}
				}
			}
		}
	}
	return false;
}

Pointer<GLX::Configuration> GLX::findConfiguration(Pointer<XScreen> pScreen, const XWindow::Configurator::Request & r){
	Pointer<Configuration> pRet;
	int nConfigs = 0;

	/*
	{
		struct t{
			int XID;
			int val;
		};
		t * p = (t *)r.list;
		int n = sizeof(r.list)/sizeof(r.list[0]);
		int i = 0;
		while(i < n){
			printf("%x %x %d\n", p[i].XID, p[i].val, (int)r.isDefault((XWindow::Configurator::FBAID)i));
			i++;
		}
	}
	*/
	GLXFBConfig * arrConfig = glXChooseFBConfig(m_pDisplay, pScreen->getScreenIndex(), (int *)(r.list), &nConfigs);

	//printf("arrConfig %p, n %d\n", arrConfig, nConfigs	);
	if(nConfigs){

		GLXFBConfig conf = arrConfig[0];
		XFree(arrConfig);
		pRet = kq_core_memory_workerRefCountedClassNew(m_pUserInterface->getMemoryWorker(), GLX::Configuration, m_pUserInterface, conf);
		//printf("GLXFBConfig Selected %p\n", conf);
	}
	return pRet;
}

void GLX::down(){
	printf("glX %p Finalized\n", this);
}


GLX::~GLX(){
	down();
}

Pointer<ui::Window> XScreen::createRootWindow(const ui::FormatSpecification * pRequests){
	return XWindow::createRootWindow(This, pRequests).castStatic<ui::Window>();
}

XScreen::XScreen(RefCounter * pRef)
	:RefCounted(pRef)
{

}

bool XScreen::up(Pointer<UserInterface> pUserInterface, ui32 iIndex){
	m_pUserInterface = pUserInterface;
	if(m_pUserInterface){
		m_iIndex = iIndex;
		return true;
	}
	return false;
}

void XScreen::down(){

}

XScreen::~XScreen(){
	down();
}

Pointer<ui::UserInterface> XScreen::getOwner(){
	return m_pUserInterface;

}

/*
struct GLXAtrribute{
	int iAttr;
	const char * pDescription;
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

*/
static const int dc = GLX_DONT_CARE;
static const int fl = False;
static const int tr = True;
static const int rt = GLX_RGBA_BIT;
static const int dt = GLX_WINDOW_BIT;
static const int nn = None;
static const XWindow::Configurator::FBAEntry defaultList[] = {
	{GLX_FBCONFIG_ID, dc},	{GLX_BUFFER_SIZE, 00},	{GLX_LEVEL, 00},	{GLX_DOUBLEBUFFER, dc},
	{GLX_STEREO, fl},	{GLX_AUX_BUFFERS, 00},	{GLX_RED_SIZE, dc},	{GLX_GREEN_SIZE, dc},
	{GLX_BLUE_SIZE, dc},	{GLX_ALPHA_SIZE, dc},	{GLX_DEPTH_SIZE, 00},	{GLX_STENCIL_SIZE, 00},
	{GLX_ACCUM_RED_SIZE, 00},	{GLX_ACCUM_GREEN_SIZE, 00},	{GLX_ACCUM_BLUE_SIZE, 00},	{GLX_ACCUM_ALPHA_SIZE, 00},
	{GLX_RENDER_TYPE, rt},	{GLX_DRAWABLE_TYPE, dt},	{GLX_X_RENDERABLE, dc}, {GLX_X_VISUAL_TYPE, dc},
	{GLX_CONFIG_CAVEAT, dc},	{GLX_TRANSPARENT_TYPE, dc},	{GLX_TRANSPARENT_INDEX_VALUE, dc},	{GLX_TRANSPARENT_RED_VALUE, dc},
	{GLX_TRANSPARENT_GREEN_VALUE, dc},	{GLX_TRANSPARENT_BLUE_VALUE, dc},	{GLX_TRANSPARENT_ALPHA_VALUE, dc},

};

XWindow::Configurator::Request::Request(){
	//printf("%lu %lu sizes\n", sizeof(list)/sizeof(int), sizeof(defaultList)/sizeof(int));
	memcpy(list, defaultList, sizeof(defaultList));
	list[nRequestAttributes].XID = None;
	list[nRequestAttributes].val = 0;
}
/*
bool XWindow::Configurator::Request::isDefault(FBAID id) const{
	if(id < nRequestAttributes){
		return (list[id].val == defaultList[id].val);
	}
	return -1;
}

int XWindow::Configurator::Request::i = 0;
*/
/*
static const char * FBANames[] = {
	{GLX_FBCONFIG_ID, "id"},	{GLX_BUFFER_SIZE, "bufsz"},	{GLX_LEVEL, "lvl"},	{GLX_DOUBLEBUFFER, "dblbuff"},
	{GLX_STEREO, "str"},	{GLX_AUX_BUFFERS, "auxbuf"},	{GLX_RED_SIZE, "r"},	{GLX_GREEN_SIZE, "g"},
	{GLX_BLUE_SIZE, "b"},	{GLX_ALPHA_SIZE, "a"},	{GLX_DEPTH_SIZE, "depth"},	{GLX_STENCIL_SIZE, "stencil"},
	{GLX_ACCUM_RED_SIZE, "ar"},	{GLX_ACCUM_GREEN_SIZE, "ag"},	{GLX_ACCUM_BLUE_SIZE, "ab"},	{GLX_ACCUM_ALPHA_SIZE, "aa"},
	{GLX_RENDER_TYPE, "rndrtyp"},	{GLX_DRAWABLE_TYPE, "drwtyp"},	{GLX_X_RENDERABLE, "xrend"},	{GLX_VISUAL_ID, "VID"},
	{GLX_X_VISUAL_TYPE, "VisTyp"},	{GLX_CONFIG_CAVEAT, "ceveat"},	{GLX_TRANSPARENT_TYPE, "trType"},	{GLX_TRANSPARENT_INDEX_VALUE, "trIDX"},
	{GLX_TRANSPARENT_RED_VALUE, "trR"},	{GLX_TRANSPARENT_GREEN_VALUE, "trG"},	{GLX_TRANSPARENT_BLUE_VALUE, "trB"},	{GLX_TRANSPARENT_ALPHA_VALUE, "trA"},
	{GLX_MAX_PBUFFER_WIDTH, "pbufW"},	{GLX_MAX_PBUFFER_HEIGHT, "pbufH"},	{GLX_MAX_PBUFFER_PIXELS, "pbufPix"},
};
*/
