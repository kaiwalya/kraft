
#include "core.hpp"
#include "stdarg.h"
#include "stdio.h"

using namespace kq::core;



#include "windows.h"

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

										//Process All Messages in queue
										continue;
									}

									SwapBuffers(hMainWindowDC);

																		
									{
										Sleep(100);
									}
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

