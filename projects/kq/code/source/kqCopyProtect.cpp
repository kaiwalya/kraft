#include "core.hpp"

#define _WIN32_WINNT 0x0602
#define WINVER 0x0602
#include "windows.h"

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;

/*
LRESULT __stdcall keyboardHook(int iCode, WPARAM wParam, LPARAM lParam){

	LRESULT lRet = CallNextHookEx(0, iCode, wParam, lParam);
	if(iCode >= 0){
		KBDLLHOOKSTRUCT * pInfo = (KBDLLHOOKSTRUCT *)lParam;
		if(pInfo->vkCode == VK_SNAPSHOT){
			lRet = 1;
			MessageBeep(MB_OK);
		}

	}
	return lRet;
};

*/

LRESULT __stdcall wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

	LRESULT lRet;

	switch(msg){
		case WM_CLOSE:
			{
				PostQuitMessage(0);
				lRet = 0;
				break;
			}
		case WM_CLIPBOARDUPDATE:
			{
				bool bBlock = true;

				HWND hOwner = GetClipboardOwner();
				if(hOwner){
					if(hWnd != hOwner){
						DWORD idProcess = 0;
						//DWORD idThread = GetWindowThreadProcessId(hOwner, &idProcess);
						if(idProcess){
							HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, idProcess);
							if(hProcess){
								wchar_t sTitle[3000];
								DWORD dwSize = sizeof(sTitle)/sizeof(sTitle[0]);
								if(QueryFullProcessImageName(hProcess, 0, sTitle, &dwSize) == TRUE){
									wchar_t * p = &sTitle[0];
									while(0 != (*p++ = towlower(*p)));
									//if(wcscmp(sTitle, L"c:\\windows\\system32\\notepad.exe") != 0){
									//	bBlock = false;
									//}
								}
									CloseHandle(hProcess);
							}
						}	
					}else{
						bBlock = false;
					};
				}

				if(bBlock){
					OpenClipboard(hWnd);
					EmptyClipboard();
					CloseClipboard();
				}

				lRet = 0;
			}
			break;
		default:
			lRet = DefWindowProc(hWnd, msg, wParam, lParam);
	};
	return lRet;
}


#include "stdio.h"


int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR resDir, int){

	//Create standard allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	{
		//Create the pooled memory allocator
		Pointer<PooledMemoryAllocator> pAllocPooled = kq_core_memory_workerRefCountedClassNew(memStd, PooledMemoryAllocator, memStd);
		MemoryWorker mem;
		pAllocPooled->getMemoryWorker(mem);

		//wchar_t ** pModules = 0;
		{
		
			int resdirlength = strlen(resDir);
			char module_file[] = "../../res/moduleinfo.txt";
			ui32 iFileNameSize = resdirlength + sizeof(module_file);
			char * pFileName = (char *)mem(0, iFileNameSize);
			if(pFileName){
				strcpy_s(pFileName, iFileNameSize, resDir);
				strcpy_s(pFileName + resdirlength,iFileNameSize - resdirlength, module_file);
				FILE * pFile = 0;
				if(fopen_s(&pFile, pFileName, "rb") == 0){
					if(fseek(pFile, 0, SEEK_END) == 0){
						ui32 iSize = ftell(pFile);
						if(iSize){
							if(fseek(pFile, 0, SEEK_SET) == 0){
								char * pSz = (char *)mem(0, iSize + 1);
								if(pSz){
									wchar_t * pSzw = (wchar_t *)mem(0, (iSize+1) * 2);
									if(pSzw){
										ui32 iReadSize = fread_s(pSz, iSize, 1, iSize, pFile);
										if(iSize == iReadSize){
											pSz[iSize] = 0;
											ui32 iByte;
											ui32 iStringCount = 0;
											for(iByte = 0; iByte < (iSize + 1); iByte++){
												pSzw[iByte] = pSz[iByte];
												switch(pSzw[iByte]){
													case L'\r':
													case L'\n':
													case L'\0':
														iStringCount++;
													
												};
												if(pSzw[iByte] == L'\n' || pSzw[iByte] == L'\r' || pSzw[iByte] == L'\0'){
													iStringCount++;
													iByte;
												}
											}
										}
										mem(pSzw, 0);
									}
									mem(pSz, 0);
								}
							};
						}
					}
					fclose(pFile);
				}
				mem(pFileName, 0);
			}
		}
		
		//HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)&keyboardHook, 0, 0);
		//if(hHook)
		{
			WNDCLASSEX wc;
			wc.cbSize = sizeof(wc);
			wc.style = CS_OWNDC;
			wc.lpfnWndProc = wndproc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = 0;
			wc.hIcon = 0;
			wc.hCursor = LoadCursor(0, IDC_APPSTARTING);
			wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wc.lpszMenuName = 0;
			wc.lpszClassName = L"KQWindowClass";
			wc.hIconSm = 0;

			ATOM regResult = RegisterClassEx(&wc);
			if(regResult){
				HWND hWnd = CreateWindow(L"KQWindowClass",  0, WS_POPUP, 0, 0, 100, 100, (HWND)0, (HMENU)0, (HINSTANCE)0, (LPVOID)0); 
				if(hWnd){
					if(AddClipboardFormatListener(hWnd) == TRUE){

						MSG msg;
						
						while(GetMessage(&msg, 0, 0, 0) == TRUE){

							TranslateMessage(&msg);
							DispatchMessage(&msg);

						};

						RemoveClipboardFormatListener(hWnd);
					}

					DestroyWindow(hWnd);
				}

				UnregisterClass(L"KQWindowClass", 0);
			}

			//UnhookWindowsHookEx(hHook);
		}

		
		
	}


	if(allocStd.getCurrentlyAllocatedByteCount()){
		//Leaks!!!
		_asm int 3;
	};

	return 1;
};
//
//int main(){WinMain(0,0,0,0);}
