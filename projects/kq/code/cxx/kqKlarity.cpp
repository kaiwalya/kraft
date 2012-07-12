#if !defined(OLD_KLARITY)

#include "WinSock2.h"
#include "Windows.h"

/*
#include "initguid.h"
#include "core.hpp"
#include "Mmdeviceapi.h"
#include "atlbase.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "memory"
#include "AudioClient.h"
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{	

	return 0;
}


#else
#include "core.hpp"

using namespace kq::core;
using namespace kq::core::memory;


#define _WIN32_WINNT 0x0502
#define WINVER 0x0502
#include "windows.h"
#include "stdio.h"

template<typename DataType>
class DataQueue{
public:
	kq::core::memory::MemoryWorker mem;
public:

	class LinkNode; class DataNode;


	kq::core::memory::Pointer<LinkNode> createLinkNode(){		
		return kq_core_memory_workerRefCountedClassNew(mem, LinkNode);
	};

	kq::core::memory::Pointer<DataNode> createDataNode(){
		return kq_core_memory_workerRefCountedClassNew(mem, DataNode);
	};

	DataQueue(kq::core::memory::MemoryWorker & worker){
		mem = worker;
		m_pTail = createLinkNode();
	};

	class DataNode{
	public:
		kq::core::memory::Pointer<DataType> m_pData;
		kq::core::memory::Pointer<LinkNode> m_pNext;
	};

	class LinkNode{
		//Lock is acquired at construction and released when data is written
		//To do a blocking read, do lock, then unlock
		HANDLE m_lock;
	public:
		kq::core::memory::Pointer<DataNode> m_pNext;
		LinkNode(){
			m_pNext = 0;
			m_lock = CreateMutex(0, 0, 0);
			WaitForSingleObject(m_lock, INFINITE);
		}

		~LinkNode(){
			CloseHandle(m_lock);
		};

		void waitForData(){
			WaitForSingleObject(m_lock, INFINITE);
			ReleaseMutex(m_lock);
		}

	};
	
	kq::core::memory::Pointer<LinkNode> m_pTail;

	void push(kq::core::memory::Pointer<DataType> pData){
		kq::core::memory::Pointer<LinkNode> m_pNewTail = kq_core_memory_workerRefCountedClassNew(mem, LinkNode);
		kq::core::memory::Pointer<DataNode> m_pNewTailData = kq_core_memory_workerRefCountedClassNew(mem, DataNode);

		m_pNewTailData->m_pNext = m_pNewTail;
		m_pNewTailData->m_pData = pData;
		
		//Push into tail		
		m_pTail->m_pNext = m_pNewTailData;
		m_pTail = m_pNewTailData->m_pNext;
		ReleaseMutex(m_pTail->m_lock);
		
		
	};

	kq::core::memory::Pointer<LinkNode> getTail(){
		kq::core::memory::Pointer<LinkNode> ret;
		ret = m_pTail;
		return ret;
	};

	class Viewer{
	public:
		kq::core::memory::Pointer<LinkNode> m_pTail;

		bool hasData(){
			return (m_pTail->m_pNext != 0);
		};

		bool getData(kq::core::memory::Pointer<DataNode> & pDataNodeOut){
			if(hasData()){
				kq::core::memory::Pointer<LinkNode> pOldTail = getTail();
				kq::core::memory::Pointer<DataNode> pDataNode = pOldTail->m_pNext;
				pDataNodeOut = pDataNode->m_pData;
				m_pTail = pDataNode->m_pNext;
				return true;
			}
			return false;
		};

		bool waitAndGetData(kq::core::memory::Pointer<DataNode> & pDataNodeOut){
			
			m_pTail->waitForData();

			kq::core::memory::Pointer<LinkNode> pOldTail = getTail();
			kq::core::memory::Pointer<DataNode> pDataNode = pOldTail->m_pNext;
			pDataNodeOut = pDataNode->m_pData;
			m_pTail = pDataNode->m_pNext;
			return true;
		};
	};

	kq::core::memory::Pointer<Viewer> createViewer(){
		kq::core::memory::Pointer<Viewer> pRet = kq_core_memory_workerRefCountedClassNew(mem, Viewer);
		pRet->m_pTail = getTail();
		return pRet;
	};

};




class WindowsConsoleOutputStream{

	HANDLE m_hOut;

	static const i32 nMaxBytesInLogMessage = 2048;
	char m_sBuffer[nMaxBytesInLogMessage];
	FILE * m_fLog;

public:

	WindowsConsoleOutputStream(HANDLE hOut = 0){
		if(hOut != 0){
			m_hOut = hOut;
		}else{
			m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		}

		m_fLog = 0;
		
	}

	i32 output(c8 * pFormat, ...){
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vsprintf(m_sBuffer, pFormat, args);
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

	i32 log(c8 * pFormat, ...){
		/*
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vsprintf_s<nMaxBytesInLogMessage>(m_sBuffer, pFormat, args);

		if(!m_fLog){
			if(0 != fopen_s(&m_fLog, "out.txt", "w")){
			}
		}

		if(m_fLog){
			fwrite(m_sBuffer, (size_t)nChars, 1, m_fLog);
			fflush(m_fLog);
		}
		*/

		i64 iRet = 0;
		/*
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
	   */


	   return (i32)iRet;
	   
	};

	i32 output(c16 * pFormat, ...){
	   va_list args;
	   va_start( args, pFormat );	   
	   i64 nChars = vswprintf(*(wchar_t (*)[nMaxBytesInLogMessage/2])m_sBuffer, pFormat, args);
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


class Globals{
public:
	class GlobalsPtr{
	public:
		Pointer<Globals> pGlobals;

		GlobalsPtr(Pointer<Globals> pGlobals){
			this->pGlobals = pGlobals;
		};

	};
private:
	struct ThreadEntryParam{

		Pointer<Globals> pGlobals;
		DWORD (Globals::* pfnThread)(void *);		
		void * pParam;

	};

	static const int kBufferInOneSec = 4;

	static DWORD __stdcall threadEntry(void * pParam){
		ThreadEntryParam * p = (ThreadEntryParam *)pParam;
		Globals * pG = p->pGlobals.operator ->();
		return (pG->*(p->pfnThread))(p->pParam);
	};

	HANDLE createThread(ThreadEntryParam * p){		
		return CreateThread(0, 0, threadEntry, p, 0, 0);
	};
		
	WAVEHDR hdrIn[kBufferInOneSec];
	WAVEHDR hdrOut[kBufferInOneSec];
	DWORD openWaveIn(void * pGPtr){
		DWORD dwRet = 0;
		GlobalsPtr * pPtr = (GlobalsPtr *) pGPtr;
		Pointer<Globals> pGlobals = pPtr->pGlobals;

		WAVEFORMATEX fmtIn;
		fmtIn.cbSize = sizeof(fmtIn);

		fmtIn.wFormatTag = WAVE_FORMAT_PCM;
		fmtIn.nSamplesPerSec = 44100;
		fmtIn.nChannels = 2;
		fmtIn.wBitsPerSample = 16;

		fmtIn.nBlockAlign = fmtIn.nChannels * fmtIn.wBitsPerSample / 8;			
		fmtIn.nAvgBytesPerSec = (WORD)(fmtIn.nSamplesPerSec * fmtIn.nBlockAlign);

		HWAVEIN hWaveIn = 0;
		MMRESULT mmres = waveInOpen(&hWaveIn, WAVE_MAPPER, &fmtIn, (DWORD_PTR)&waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if(mmres == MMSYSERR_NOERROR){
			pGlobals->pConsole->output("Wave In Device opened\n");
			pGlobals->hWaveIn = hWaveIn;
			
			memset(hdrIn, 0, sizeof(hdrIn));
			for(i32 i = 0; i < sizeof(hdrIn)/sizeof(hdrIn[0]); i++){				
				DWORD nBytes = fmtIn.nSamplesPerSec * fmtIn.nBlockAlign/kBufferInOneSec;
				hdrIn[i].lpData = (LPSTR)mem(0, nBytes);
				hdrIn[i].dwBufferLength = nBytes;
				hdrIn[i].dwUser = i;
				
				if(MMSYSERR_NOERROR == waveInPrepareHeader(hWaveIn, &hdrIn[i], sizeof(hdrIn[i])))
					if(MMSYSERR_NOERROR == waveInAddBuffer(hWaveIn, &hdrIn[i], sizeof(hdrIn[i]))){
						
					};
			}			
			pGlobals->pConsole->output("Wave In Ready\n");
			
		}else{
			pGlobals->pConsole->output("Cannot Open Device\n");
			dwRet = 1;
		};

		return dwRet;
	};

	DWORD openWaveOut(void * pGPtr){
		DWORD dwRet = 0;
		GlobalsPtr * pPtr = (GlobalsPtr *) pGPtr;
		Pointer<Globals> pGlobals = pPtr->pGlobals;

		WAVEFORMATEX fmtIn;
		fmtIn.cbSize = sizeof(fmtIn);

		fmtIn.wFormatTag = WAVE_FORMAT_PCM;
		fmtIn.nSamplesPerSec = 44100;
		fmtIn.nChannels = 2;
		fmtIn.wBitsPerSample = 16;

		fmtIn.nBlockAlign = fmtIn.nChannels * fmtIn.wBitsPerSample / 8;			
		fmtIn.nAvgBytesPerSec = (WORD)(fmtIn.nSamplesPerSec * fmtIn.nBlockAlign);

		HWAVEOUT hWaveOut = 0;
		MMRESULT mmres = waveOutOpen(&hWaveOut, WAVE_MAPPER, &fmtIn, (DWORD_PTR)&waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if(mmres == MMSYSERR_NOERROR){
			pGlobals->hWaveOut = (HWAVEOUT)hWaveOut;
			pGlobals->pConsole->output("Wave Out Device Opened\n");

			memset(hdrOut, 0, sizeof(hdrIn));			
			for(i32 i = 0; i < sizeof(hdrOut)/sizeof(hdrOut[0]); i++){
				DWORD nBytes = fmtIn.nSamplesPerSec * fmtIn.nBlockAlign/kBufferInOneSec;
				hdrOut[i].lpData = (LPSTR)mem(0, nBytes);
				hdrOut[i].dwBufferLength = nBytes;
				hdrOut[i].dwUser = i;				
				if(MMSYSERR_NOERROR == waveOutPrepareHeader(hWaveOut, &hdrOut[i], sizeof(hdrOut[i])))
					{
						int j = 0;
						j++;
					};
			}			
			
			pGlobals->pConsole->output("Wave Out Ready\n");
		}else{
			pGlobals->pConsole->output("Cannot Open Device\n");
			dwRet = 1;
		};

		return dwRet;
	};
	

public:


	//Modified by Constructiong/Destruction
	MemoryWorker mem;
	//Pointer<Globals> pThis;

	//Modified by up/down
	Pointer<WindowsConsoleOutputStream> pConsole;
	HWAVEIN hWaveIn;
	HWAVEOUT hWaveOut;

	Globals(MemoryWorker &worker):mem(worker){
		pConsole = 0;
		hWaveIn = 0;
		hWaveOut = 0;
	}

	bool up(Pointer<Globals> pGlobals){

		GlobalsPtr globalsPtr(pGlobals);
		HANDLE hWaveThreads[2];

		
		ThreadEntryParam p[2];
		p[0].pGlobals = pGlobals;
		p[0].pfnThread = (&Globals::openWaveIn);
		p[0].pParam = &globalsPtr;
		
		hWaveThreads[0] = createThread(&p[0]);
		
		p[1] = p[0];
		p[1].pfnThread = (&Globals::openWaveOut);
		hWaveThreads[1] = createThread(&p[1]);

		if(hWaveThreads[0] && hWaveThreads[1]){
			WaitForMultipleObjects(2, hWaveThreads, TRUE, INFINITE);
			return true;
		}
		return false;
	};

	bool down(){
		if(hWaveIn){
			waveInClose(hWaveIn);
			hWaveIn = 0;
		}
		if(hWaveOut){
			waveOutClose(hWaveOut);
			hWaveOut = 0;
		};		
		return true;
	}

	~Globals(){
		down();
	};

	static void __stdcall waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
		hwi;
		uMsg;
		dwInstance;
		dwParam1;
		dwParam2;

		if(uMsg == WIM_DATA){
			Globals * pGlobals = (Globals *)dwInstance;

			WAVEHDR * pHdrIn = (WAVEHDR *)dwParam1;
			DWORD bufferIndex = pHdrIn->dwUser;
			WAVEHDR * pHdrOut = &pGlobals->hdrOut[bufferIndex];

			DWORD nShorts = pHdrIn->dwBytesRecorded/2;
			DWORD iShorts;

			double fPowerIn = 0;
			i16 * pSampleIn = (i16 *)pHdrIn->lpData;
			for(iShorts = 0; iShorts < nShorts; iShorts += 2){
				double l = pSampleIn[iShorts];
				double r = pSampleIn[iShorts + 1];
				l/= 32786.0;
				r/= 32786.0;

				fPowerIn += ((l*l + r*r)/(2 * nShorts));
			};


			if(!bufferIndex){
				pGlobals->pConsole->output("\n");
			};
						
			pGlobals->pConsole->output("%f ", fPowerIn);
			

			memset(pHdrOut->lpData, 0, pHdrIn->dwBytesRecorded);
			//memcpy(pHdrOut->lpData, pHdrIn->lpData, pHdrIn->dwBytesRecorded);
			
			i16 * pSample = (i16 *)pHdrOut->lpData;
			if(bufferIndex == 0){
				pSample[0] = 32767;
				pSample[1] = -32768;
			}

			/*
			while(nShorts){
				i16 i1, i2;
				i1 = *pSample;
				i2 = *(pSample + 1);

				*pSample = -(i1 + i2)/2;
				*(pSample + 1) = -(i1 + i2)/2;
				nShorts--;
			};
			*/
			pHdrOut->dwBytesRecorded = pHdrIn->dwBytesRecorded;
			pHdrOut->dwLoops = 1;

			MMRESULT res;
			if(MMSYSERR_NOERROR == (res = waveOutWrite(pGlobals->hWaveOut, pHdrOut, sizeof(*pHdrOut)))){
				int j;
				j = 0;
			}
			else{
				res = 0;
			}
		}
	}

	static void __stdcall waveOutProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
		hwi;
		uMsg;
		dwInstance;
		dwParam1;
		dwParam2;

		
		if(uMsg == WOM_DONE){
			Globals * pGlobals = (Globals *)dwInstance;

			WAVEHDR * pHdrOut = (WAVEHDR *)dwParam1;
			DWORD bufferIndex = pHdrOut->dwUser;
			WAVEHDR * pHdrIn = &pGlobals->hdrIn[bufferIndex];
			waveInAddBuffer(pGlobals->hWaveIn, pHdrIn, sizeof(*pHdrIn));
		}
	}
};




int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR /*resDir*/, int){

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

	
	//Create standard allocator
	pConsole->output("Creating Standard Allocator\n");
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	//Create the pooled memory allocator
	pConsole->output("Creating Pooled Allocator\n");
	PooledMemoryAllocator allocPooled(memStd);
	MemoryWorker mem;
	allocPooled.getMemoryWorker(mem);


	{
		//Create Globals
		Pointer<Globals> pGlobals = kq_core_memory_workerRefCountedClassNew(mem, Globals, mem);
		if(pGlobals){
			pGlobals->pConsole = kq_core_memory_workerRefCountedClassNew(mem, WindowsConsoleOutputStream);
			if(pGlobals->up(pGlobals)){

				waveInStart(pGlobals->hWaveIn);
				bool b = true;
				while(b){
					Sleep(10);
				};
				pGlobals->down();
			}
		}
	}



	return 0;
};
#endif