
#include "core.hpp"
#include "stdarg.h"
#include "stdio.h"

using namespace kq::core;



#include "windows.h"
#include "math.h"
#include "gl/gl.h"

class ITicker{
public:
    virtual ui64 getTickCount() const = 0;
    virtual ui64 getTicksPerSecond() const = 0;
    virtual ~ITicker(){}
};

class IClock{
public:
    virtual i64 waitTillNextCycle() const = 0;
};

class WindowsAPITicker:public ITicker{

    ui64 iTicksPerSecond;   

public:
    WindowsAPITicker(){

        LARGE_INTEGER li;
        if(QueryPerformanceFrequency(&li)){           
            iTicksPerSecond = li.QuadPart;           
        }else{
            iTicksPerSecond = 0;
           
        }
    }

    ui64 getTicksPerSecond() const{
        return iTicksPerSecond;
    }

    ui64 getTickCount() const{

        LARGE_INTEGER li;
        ui64 iTicks;
        if(QueryPerformanceCounter(&li)){
            iTicks = li.QuadPart;
        }else{
            iTicks = 0;
        }
       
        return iTicks;       
    }
};

class IStopWatch:public ITicker{
public:
    virtual void reset() = 0;
};

class StopWatch:public IStopWatch{
	kq::core::memory::Pointer<const ITicker> m_pProvider;
    ui64 m_iInitialTime;
public:
    StopWatch(kq::core::memory::Pointer<const ITicker> pTimeProvider):m_pProvider(pTimeProvider){
        reset();
    }

    void reset(){
        m_iInitialTime = m_pProvider->getTickCount();
    };

    ui64 getTickCount() const{
        return m_pProvider->getTickCount() - m_iInitialTime;
    }

    ui64 getTicksPerSecond() const{
        return m_pProvider->getTicksPerSecond();
    };
};

class ISleeper{
public:
    virtual ui64 milliSleep(ui64 nMilli) const = 0;
};

class WindowsAPISleeper :public ISleeper{
public:
    ui64 milliSleep(ui64 nMilli) const{
        Sleep((DWORD)nMilli);
        return (DWORD)nMilli;
    };
};

class Clock:public IClock{

    kq::core::memory::Pointer<IStopWatch> m_pStopWatch;
    kq::core::memory::Pointer<const ISleeper> m_pSleeper;   
    double m_dMilliSecondsPerCycle;

public:
    Clock(double dTargetCyclesPerSecond, kq::core::memory::Pointer<IStopWatch> pStopWatch, kq::core::memory::Pointer<const ISleeper> pSleeper){
        m_dMilliSecondsPerCycle = 1000.0 / dTargetCyclesPerSecond;
        m_pStopWatch = pStopWatch;
        m_pSleeper = pSleeper;

    };

    i64 waitTillNextCycle() const{
        double dTicksPerSecond = (double)m_pStopWatch->getTicksPerSecond();
        double dConsumedTicksThisCycle = (double)m_pStopWatch->getTickCount();
        

        double dConsumedMilliSecondsThisCycle = 1000.0 * dConsumedTicksThisCycle / dTicksPerSecond;
        double dRemainingMilliSecondsThisCycle = m_dMilliSecondsPerCycle - dConsumedMilliSecondsThisCycle;

        if(dRemainingMilliSecondsThisCycle >= 1.0){
            m_pSleeper->milliSleep((ui64)dRemainingMilliSecondsThisCycle);           
        }
		m_pStopWatch->reset();
        return (i64)dRemainingMilliSecondsThisCycle;

    }

};



class WindowsConsoleOutputStream{

	HANDLE m_hOut;

	static const i32 nMaxBytesInLogMessage = 2048;
	char m_sBuffer[nMaxBytesInLogMessage];

public:

	WindowsConsoleOutputStream(HANDLE hOut = 0){
		if(hOut != 0){
			m_hOut = hOut;
		}else{
			m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		}
	}

	i32 output(c8 * pFormat, ...){
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vsprintf_s<nMaxBytesInLogMessage>(m_sBuffer, pFormat, args);
	   //Add null termination
	   nChars++;
	   i64 iRet;
	   if(nChars < nMaxBytesInLogMessage){
		   DWORD nCharsOut = 0;
		   //Remove null termination from count
		   nChars--;
		   if(WriteConsoleA(m_hOut, m_sBuffer, (DWORD)nChars, &nCharsOut, 0) == TRUE){
			   iRet = nCharsOut;
		   }else{
			   iRet = -1;
		   };
	   }else{
		   iRet = -2;
	   }

	   return (i32)iRet;
	   
	};

	i32 output(c16 * pFormat, ...){
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vswprintf_s<nMaxBytesInLogMessage/2>(*(wchar_t (*)[nMaxBytesInLogMessage/2])m_sBuffer, pFormat, args);
	   //Add null termination
	   nChars++;
	   i64 iRet;
	   if(nChars < nMaxBytesInLogMessage){
		   DWORD nCharsOut = 0;
		   //Remove null termination from count
		   nChars--;
		   if(WriteConsoleW(m_hOut, m_sBuffer, (DWORD)nChars, &nCharsOut, 0) == TRUE){
			   iRet = nCharsOut;
		   }else{
			   iRet = -1;
		   };
	   }else{
		   iRet = -2;
	   }

	   return (i32)iRet;
	   
	};


};


struct VertexTable{
	ui32 nVertices;
	double (* m_pVertex)[3];
};

struct FaceTable{
	ui32 nFaces;
	ui32 (* m_pFace)[3];
};



struct SceneGraphNode{

	kq::core::memory::Pointer<SceneGraphNode> m_pNextSibling;
	kq::core::memory::Pointer<SceneGraphNode> m_pFirstChild;
	
	
	enum OperationType{

		
		otNone = 0,
		otMove = 1,
		otRender = 2,
		
	};

	virtual void operate(OperationType ot, ...){
		ot;
	}

	virtual void postOperate(OperationType ot, ...){
		ot;
	}



};


struct Model{
	VertexTable tabV;
	FaceTable tabF;	
};

struct SceneGraphNode_Root: public SceneGraphNode{


	struct StackEntry{
		kq::core::memory::Pointer<StackEntry> pPrev;

		bool bProcessed;
		kq::core::memory::Pointer<SceneGraphNode> pNode;		

	};


	void operate(OperationType ot, ...){

		
		kq::core::memory::Pointer<StackEntry> pTop(new kq::core::memory::RefCounter( new StackEntry));
		pTop->pNode = m_pFirstChild;
		pTop->bProcessed = false;

		
			
		while(pTop){
			if(pTop->pNode){
				pTop->pNode->operate(ot);
								

				//push child
				kq::core::memory::Pointer<StackEntry> pNewTop(new kq::core::memory::RefCounter( new StackEntry));
				pNewTop->pNode = pTop->pNode->m_pFirstChild;
				pNewTop->pPrev = pTop;
				pTop = pNewTop;
			}else{
				//Last child reached,

				//pop child
				pTop = pTop->pPrev;

				//go to sibling
				if(pTop){
					//We can assume that if we pop, there is always a node if there is a valid top
					pTop->pNode->postOperate(ot);
					pTop->pNode = pTop->pNode->m_pNextSibling;
					pTop->bProcessed = false;
				}
				
			}
		}
		

	}
};

struct SceneGraphNode_OpenGL_First: public SceneGraphNode{
	
	void render(){
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void operate(OperationType ot, ...){

		switch(ot){
			case otRender:
				render();
				break;
		}

	}
};


struct SceneGraphNode_OpenGL_Transform: public SceneGraphNode{

	double d;

	SceneGraphNode_OpenGL_Transform(){
		d = 0.0;
	}

	
	void beginRender(){
		glPushMatrix();
		
		glRotated(360 * sin(d), 0, 0, 1);
		glScaled(sin(3 * d), cos(7 *d), 0.0);
		glTranslated(0.5 * cos(5 * d), 0.5 * sin(3 * d), 0.0);
	}

	void endRender(){
		glPopMatrix();
	}

	void move(){		
		d = d + 0.01;
	}

	void operate(OperationType ot, ...){

		switch(ot){
			case otRender:
				beginRender();
				break;			
			case otMove:
				move();
				break;
		}

	}

	void postOperate(OperationType ot, ...){

		switch(ot){
			case otRender:				
				endRender();
				break;
		}

	}
};


struct SceneGraphNode_OpenGL_Model:SceneGraphNode{

	kq::core::memory::Pointer<Model> pModel;

	void render(){
		VertexTable &tabV = pModel->tabV;
		FaceTable &tabF = pModel->tabF;

		

		ui32 iFace, nFaces;
		nFaces = tabF.nFaces;


	
		ui32 (*pFace)[3] = tabF.m_pFace;
		double (*pVertex)[3] = tabV.m_pVertex;

		glBegin(GL_TRIANGLES);
		for(iFace = 0; iFace < nFaces; iFace++){																								

			glColor4d(1, 0, 0, 0.6);
			glVertex3dv(pVertex[pFace[0][0]]);
			glColor4d(0, 1, 0, 0.6);
			glVertex3dv(pVertex[pFace[0][1]]);
			glColor4d(0, 0, 1, 0.6);
			glVertex3dv(pVertex[pFace[0][2]]);

			
			pFace++;
		}
		glEnd();
		


	}

	void operate(OperationType ot, ...){

		switch(ot){
			case otRender:
				render();
				break;
		}

	}
};



double TriangleModelVertexTableVertexData[3][3] = {{0, 0, 0}, {0.5, 0, 0}, {0, 0.5, 0}};
ui32 TriangleModelVertexTableVertexCount = sizeof(TriangleModelVertexTableVertexData)/sizeof(TriangleModelVertexTableVertexData[0]);

ui32 TriangleModelFaceTableFaceData[1][3] = {0, 1, 2};
ui32 TriangleModelFaceTableFaceCount = sizeof(TriangleModelFaceTableFaceData)/sizeof(TriangleModelFaceTableFaceData[0]);

Model TriangleModel = {{TriangleModelVertexTableVertexCount, TriangleModelVertexTableVertexData}, {TriangleModelFaceTableFaceCount, TriangleModelFaceTableFaceData}};

LRESULT WINAPI wndproc(HWND h, UINT m, WPARAM w, LPARAM l){
	LRESULT lRet = 0;	
	switch(m){
	case WM_CREATE:
		lRet = 0;		
		break;
	case WM_SETCURSOR:
		SetCursor((HCURSOR)LoadCursor(0,IDC_ARROW));
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		lRet = DefWindowProc(h, m, w, l);
	}

	return lRet;
}

void * operator new (size_t nBytes , void * p){
	nBytes = 0;	
	return p;
}

void * operator new[] (size_t nBytes , void * p){
	nBytes = 0;
	return p;
}

void operator delete (void * pObj, void * pMem){	
	pMem = 0;
	pObj = 0;

}

void operator delete[] (void * pObj, void * pMem){	
	pMem = 0;
	pObj = 0;

}




#define kq_core_memory_workerNew(memworker, classname, ...) (new (memworker(0, sizeof(classname))) classname __VA_ARGS__)
#define kq_core_memory_workerDelete(memworker, classname, obj) (obj->~classname());(memworker(obj, 0))
#define kq_core_memory_workerRefCountedNew(memworker, classname, ...) (kq_core_memory_workerNew(memworker, kq::core::memory::RefCounter, (kq_core_memory_workerNew(memworker, classname, __VA_ARGS__)) ))

class C{
public:
	C(){
		int j;
		j = 0;
	}
	~C(){
		int j;
		j = 0;
	}
};


namespace ndv{

	template<typename DimensionCoordinate>
	struct NDimensionalVector{	
		DimensionCoordinate * pOffsets;
	};

	template<typename DimensionCoordinate>
	struct NDimensionalSegment{
		NDimensionalVector<DimensionCoordinate> * m_pStart;
		NDimensionalVector<DimensionCoordinate> * m_pOffsets;
	};

	template<typename DimensionCountType, typename DimensionCoordinate>
	struct NDimensionalView{
		ui8 * m_pStart;

		//Number of dimensions
		DimensionCountType nDimensions;

		//The size of the smallest unit in the lowest dimension
		DimensionCoordinate nBytesPerUnit;

		//These are the bytes per dimension, used to calculate offsets for the next row in the same dimension
		//thus to move from A[z][y][x] to A[z+1][y+2][x+3] would be an offset of (arr[2] + 2*arr[1] + 3*arr[0])
		NDimensionalVector jumps;

	};

	template<typename DimensionCountType, typename DimensionCoordinate>
	DimensionCoordinate nd_memcpy(NDimensionalView<DimensionCoordinate, DimensionCoordinate> * pDest, NDimensionalView<DimensionCoordinate, DimensionCoordinate> * pSrc, NDimensionalVector<DimensionCoordinate> nBytes);	

	template<typename DimensionCountType, typename DimensionCoordinate>
	DimensionCoordinate nd_memset(NDimensionalView<DimensionCoordinate, DimensionCoordinate> * pDest, ui8 * pBytesSrc, NDimensionalVector<DimensionCoordinate> nBytes);	
}

/*
struct Box{

	//Contents of this box;
	union{
		//Data content
		ui8 * pData;

		//Other boxes
		Block * pChildren;
	};

	ui8 nDimensions;

	//Bits determine the number of children there are, (nBits^2^nDimensions)
	ui8 nBitsIndexedPerDimension;

	//Parent
	Box * m_pParent;
	//Location in parent, array of number of dimensions;
	ui8 * m_pIndices;
};


class Graph{
public:

	//The NodeID, it is just a stream of bits.

	typedef ui32 NodeIDLengthType;

	struct NodeID{
		void * m_pID;
		NodeIDLengthType iStartBit;
		NodeIDLengthType iEndBit;
	};

	
	Square * m_pRoot;

	typedef NodeID * PNodeID;
	typedef bool (*PFNNodeIDAccessor)(void * pData, NodeID * pID);

	typedef NodeID NodeIDListZ;
	typedef struct {PNodeID pIDs; ui32 nIDs;} NodeIDListN;
	typedef struct {PFNNodeIDAccessor pfnAccessor; void * pData;} NodeIDListF;

	enum NodeIDList{
		enum NodeIDListType{
			NodeIDList_None,
			NodeIDList_NullTerminated,
			NodeIDList_Counted,
			NodeIDList_IteratorFunction,
		};

		NodeIDListType typ;

		union{
			NodeIDListZ z;
			NodeIDListN N;
			NodeIDListF F;
		};
	};
	
	i32 connect(NodeIDList *listSrc, NodeIDList *listDest);
	i32 disconnect(NodeIDList *listSrc, NodeIDList *listDest);
	i32 search(NodeIDList source, NodeIDList dest, NodeIDList * pOut);

};

*/


int main(int /*argc*/, char **){

	kq::core::memory::MemoryWorker mem;
	kq::core::memory::StandardLibraryMemoryAllocator a;
	a.getMemoryWorker(mem);


	C * pC = kq_core_memory_workerNew(mem, C);
	if(pC){
		kq_core_memory_workerDelete(mem, C, pC);
	}

	kq::core::memory::Pointer<C>( kq_core_memory_workerRefCountedNew(mem, C) ) ;

	//WinMain(GetModuleHandle(0), 0, 0, 0);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/){


	if(!GetStdHandle(STD_OUTPUT_HANDLE)){
		if(AttachConsole(ATTACH_PARENT_PROCESS) == FALSE){
			if(AllocConsole() == FALSE){
				return -1;
				
			}
		}	
	}

	WindowsConsoleOutputStream console;
	WindowsConsoleOutputStream * pConsole = &console;
	pConsole->output("Console Init...\tAttached.\n");

	kq::core::memory::Pointer<ITicker> pTicker = new kq::core::memory::RefCounter(new WindowsAPITicker());
	kq::core::memory::Pointer<IStopWatch> pStopWatch = new kq::core::memory::RefCounter(new StopWatch( pTicker ));
	kq::core::memory::Pointer<IClock> pClock(new kq::core::memory::RefCounter(new Clock(80, new kq::core::memory::RefCounter(new StopWatch( pTicker )), new kq::core::memory::RefCounter(new WindowsAPISleeper))));


	DISPLAY_DEVICE dd = {0};
	dd.cb = sizeof(dd);
	DEVMODE dm = {0};
	dm.dmSize = sizeof(dm);

	DEVMODE dmCurrent = {0};
	dmCurrent.dmSize = sizeof(dmCurrent);

	int iDisplayDevice = 0;

	
	DWORD dwCheck = DISPLAY_DEVICE_ACTIVE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP | DISPLAY_DEVICE_PRIMARY_DEVICE;	
	
	bool bRequestedResolutionSet = false;

	while( !bRequestedResolutionSet && EnumDisplayDevices(0, iDisplayDevice, &dd, 0) ){

		EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dmCurrent);
		
		//Search of displays which are relevant
		if((dd.StateFlags & dwCheck) == dwCheck){

			pConsole->output(L"Using Display...\t%s.\n", dd.DeviceString);			

			int iDevmode = 0;
			while(EnumDisplaySettings(dd.DeviceName, iDevmode, &dm)){
				
				if(dm.dmBitsPerPel >= 32){
					if(
						dm.dmPelsWidth == dmCurrent.dmPelsWidth
						&& dm.dmPelsHeight == dmCurrent.dmPelsHeight
						&& dm.dmBitsPerPel == dmCurrent.dmBitsPerPel
						&& dm.dmDisplayFrequency == dmCurrent.dmDisplayFrequency
					){

						//pConsole->output(L"***", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel, dm.dmDisplayFrequency);
						pConsole->output(L"Using Current Resolution...\t%4dx%4d %2dbits @ %2dHz\n", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel, dm.dmDisplayFrequency);					
						pConsole->output("Switching To FullScreen...\t");
						if((ChangeDisplaySettingsEx(dd.DeviceName, &dmCurrent, 0, CDS_FULLSCREEN, 0) == DISP_CHANGE_SUCCESSFUL)){
							bRequestedResolutionSet = true;
							pConsole->output("Done.\n");
							break;
						}else{
							pConsole->output("Failed!\n");
						}
					}										
				}
				iDevmode++;
			}
		}
		iDisplayDevice++;
	}			
	

	if(bRequestedResolutionSet){
		WNDCLASSEX wc;
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = wndproc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = 0;
		wc.hCursor = LoadCursor(0, IDC_APPSTARTING);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = L"KQWindowClass";
		wc.hIconSm = 0;


		pConsole->output(L"Registering Class For main window(%s)...\t", wc.lpszClassName);
		ATOM regResult = RegisterClassEx(&wc);
		if(regResult){


			LPWSTR lpszWindowName = L"Game Window";
			pConsole->output(L"%d\nCreating Window(%s::%s)...", regResult, wc.lpszClassName, lpszWindowName);

			HWND hMainWindow = CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, lpszWindowName, WS_POPUP, 0, 0, dmCurrent.dmPelsWidth, dmCurrent.dmPelsHeight, 0, 0, hInstance, 0);
			if(hMainWindow){
				pConsole->output("%x\nGetting Device Context...\t", hMainWindow);
				HDC hMainWindowDC = GetDC(hMainWindow);
				if(hMainWindowDC){
					pConsole->output("%x\n", hMainWindowDC);
					PIXELFORMATDESCRIPTOR pfd = {0};
					pfd.nSize = sizeof(pfd);
					pfd.nVersion = 1;
					pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
					pfd.iPixelType = PFD_TYPE_RGBA;
					pfd.cColorBits = 24;
					pfd.cDepthBits = 16;
					pfd.iLayerType = PFD_MAIN_PLANE;

					pConsole->output("Trying to set pixel format...\t");

					int iPixelFormat = ChoosePixelFormat(hMainWindowDC, &pfd);
					if(iPixelFormat && (SetPixelFormat(hMainWindowDC, iPixelFormat, &pfd) == TRUE)){
						pConsole->output("Set\nCreating OpenGL Context...\t");
						HGLRC hGL = wglCreateContext(hMainWindowDC);
						if(hGL){
							pConsole->output("%x\n", hGL);
							HGLRC hGLOld = wglGetCurrentContext();
							if((wglMakeCurrent(hMainWindowDC, hGL) == TRUE)){
								MSG msg;
								bool bContinue = true;
								pConsole->output("Showing Main Window...\nEntering Game loop...\n");

								ShowWindow(hMainWindow, SW_MAXIMIZE);

								kq::core::memory::Pointer<SceneGraphNode> pRoot;

								LARGE_INTEGER iCountOld;
								iCountOld.QuadPart = 0;
								while(bContinue){
									BOOL bMsg = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
									if(bMsg == TRUE){
										switch(msg.message){
											case WM_QUIT:
												pConsole->output("Got Quit Message...\n");
												bContinue = false;					
												break;
											default:
												TranslateMessage(&msg);
												DispatchMessage(&msg);
												break;
										};

										//Go back and process all messages in queue before proceeding
										continue;
									}

									
									{

										

										if(!pRoot){
											pRoot = (new kq::core::memory::RefCounter (new SceneGraphNode_Root));

											kq::core::memory::Pointer<SceneGraphNode_OpenGL_First> pOpenGL = new kq::core::memory::RefCounter ( new SceneGraphNode_OpenGL_First);

											kq::core::memory::Pointer<SceneGraphNode_OpenGL_Model> pModel = new kq::core::memory::RefCounter (new SceneGraphNode_OpenGL_Model);
											pModel->pModel = new kq::core::memory::RefCounter ( new Model);
											pModel->pModel->tabF = TriangleModel.tabF;
											pModel->pModel->tabV = TriangleModel.tabV;

											kq::core::memory::Pointer<SceneGraphNode_OpenGL_Transform> pTransform = new kq::core::memory::RefCounter (new SceneGraphNode_OpenGL_Transform);
											

											kq::core::memory::Pointer<SceneGraphNode_OpenGL_Model> pModel2 = new kq::core::memory::RefCounter (new SceneGraphNode_OpenGL_Model);
											pModel2->pModel = new kq::core::memory::RefCounter ( new Model);
											pModel2->pModel->tabF = TriangleModel.tabF;
											pModel2->pModel->tabV = TriangleModel.tabV;

											pTransform->m_pFirstChild = pModel2.castStatic<SceneGraphNode>();

											pOpenGL->m_pFirstChild = pTransform.castStatic<SceneGraphNode>();
											pTransform->m_pNextSibling = pModel.castStatic<SceneGraphNode>();


											pRoot->m_pFirstChild = pOpenGL.castStatic<SceneGraphNode>();

											pStopWatch->reset();
											
										}

										
										double dSecondsSinceLastReset = (double)pStopWatch->getTickCount() / (double)pStopWatch->getTicksPerSecond();
										pStopWatch->reset();

										pRoot->operate(SceneGraphNode::otMove, dSecondsSinceLastReset);
										pRoot->operate(SceneGraphNode::otRender);


										
									}

									SwapBuffers(hMainWindowDC);
									pClock->waitTillNextCycle();								
								}

								wglMakeCurrent(hMainWindowDC, hGLOld);
							}else{
								pConsole->output("Failed to set current context");
							}
							wglDeleteContext(hGL);
						}else{
							pConsole->output("Failed to create OpenGl Context");
						}

					}else{
						pConsole->output("Failed to fix pixel format\n");
					}

					ReleaseDC(hMainWindow, hMainWindowDC);
				}else{
					pConsole->output("Failed to get DC\n");
				}

				DestroyWindow(hMainWindow);
			}else{
				pConsole->output("Failed to Create window\n");
			}

			UnregisterClass(wc.lpszClassName, hInstance);
		}else{
			pConsole->output("Failed to Register Class\n");
		}
	}else{
		pConsole->output("Couldnot Set Requested Resolution\n");
	}

	pConsole->output("Restoring Default Display Settings\n");
	ChangeDisplaySettingsEx(0, 0, 0, 0, 0);

	pConsole->output("Detaching Console\n");
	FreeConsole();
	return 0;
}
