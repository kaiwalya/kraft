
#include "core.hpp"
#include "stdarg.h"
#include "stdio.h"

using namespace kq::core;



#include "windows.h"


class RefCounter{
public:
    void * object;
    ui32 count;
    bool (*AtLast)(RefCounter *);

    RefCounter(void * object = 0, ui32 count = 0, bool (*AtLast)(RefCounter *) = 0){   
        this->object = object;
        this->count = count;
        this->AtLast = AtLast;
    }


    RefCounter(void * object, bool (*AtLast)(RefCounter *)){

        this->count = 0;
        this->object = object;
        this->AtLast = AtLast;
    }

};

static RefCounter nullCounter;

template<typename t>
class Pointer{
    RefCounter * m_pRefCounter;
    void * m_pBufferedObject;

    void setReference(RefCounter * pRefCounter){
        m_pRefCounter = pRefCounter;
        m_pBufferedObject = pRefCounter->object;
    };

    void attach(RefCounter * pRefCounter){
        setReference(pRefCounter);

        m_pRefCounter->count++;       
    };
   
    void detach(){
        m_pRefCounter->count--;
        if(!m_pRefCounter->count && m_pRefCounter->object){
            if( !(m_pRefCounter->AtLast) || !(*(m_pRefCounter->AtLast))(m_pRefCounter) ){
                delete (t *)m_pRefCounter->object;
                delete m_pRefCounter;
            }
        }
        setReference(&nullCounter);

    }

   
public:

    Pointer(RefCounter * pRefCounter){
        attach(pRefCounter);
    };

    Pointer(const Pointer<t> & pointer){
        attach(pointer.m_pRefCounter);
    };

    Pointer(){
        attach(&nullCounter);
    }

    ~Pointer(){
        detach();
    }

    Pointer<t> & operator = (const Pointer<t> & oprand){
        if(m_pRefCounter->object != oprand.m_pRefCounter->object){
            detach();
            attach(oprand.m_pRefCounter);           
        }
        return *this;
    };

    operator Pointer<const t>()const{

        Pointer<const t> ret(m_pRefCounter);
        return ret;
    }

    bool operator == (const Pointer<t> & oprand) const{
        return (oprand.m_pRefCount->object == m_pBufferedObject);

    }

    bool operator != (const Pointer<t> & oprand) const{
        return (oprand.m_pRefCount->object != m_pBufferedObject);
    }

    t * operator ->()const {
        return (t *)(m_pBufferedObject);
    };

};

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
    Pointer<const ITicker> m_pProvider;
    ui64 m_iInitialTime;
public:
    StopWatch(Pointer<const ITicker> pTimeProvider):m_pProvider(pTimeProvider){
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

    Pointer<IStopWatch> m_pStopWatch;
    Pointer<const ISleeper> m_pSleeper;   
    double m_dMilliSecondsPerCycle;

public:
    Clock(double dTargetCyclesPerSecond, Pointer<IStopWatch> pStopWatch, Pointer<const ISleeper> pSleeper){
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

int main(int /*argc*/, char **){
	WinMain(GetModuleHandle(0), 0, 0, 0);
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

	Pointer<IClock> pClock(new RefCounter(new Clock(30, new RefCounter(new StopWatch(new RefCounter(new WindowsAPITicker()))), new RefCounter(new WindowsAPISleeper))));


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

