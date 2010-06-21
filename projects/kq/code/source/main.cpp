
#include "flows.hpp"
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



#include <new>



/*
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
*/

struct BitBox_16x16{
	ui8 data[32];
	ui8 nBitsSet;
	
	struct Address{
		union{
			ui8 addrByte;
			struct {
				ui8 bitOffset:3;
				ui8 byteIndex:5;
				
			} addrSplit;
			struct{
				ui8 y:4;
				ui8 x:4;
			}addrNode;
		};

		static bool test(){
			
			//Check if ordering inside the struct is as per assumptions
			Address s;
			ui8 addrSize = sizeof(s);
			if(addrSize != 1){
				_asm int 3;
				return false;
			}

			//Check that the bits are orders as expected
			s.addrByte = 1;
			if(s.addrSplit.bitOffset != 1 || s.addrNode.y != 1){
				_asm int 3;
				return false;
			}
			
			return true;
		}
	};

	bool up(){
		memset(&data, 0, sizeof(data));
		nBitsSet = 0;
		return true;
	};

	bool down(){
		return true;
	};

	ui32 setBit(Address s){
		ui8 old = data[s.addrSplit.byteIndex];
		data[s.addrSplit.byteIndex] |= (ui8)((ui8)1 << s.addrSplit.bitOffset);
		if(old != data[s.addrSplit.byteIndex]){
			nBitsSet++;
		};

		return nBitsSet;
	};

	ui32 resetBit(Address s){
		ui8 old = data[s.addrSplit.byteIndex];
		data[s.addrSplit.byteIndex] &= (ui8)(~((ui8)1 << s.addrSplit.bitOffset));
		if(old != data[s.addrSplit.byteIndex]){
			nBitsSet--;
		};
		return nBitsSet;
	};

	bool getBit(Address s) const{
		return ((data[s.addrSplit.byteIndex] & (ui8)((ui8)1 << s.addrSplit.bitOffset)) != 0);
	};

	ui8 getSetCount() const{
		return nBitsSet;
	};
	
	static bool test(){
		
		//Check inner classes
		if(!Address::test()){
			return false;
		}

		
		BitBox_16x16 box;

		//Check if initialization is ok
		{
			box.up();
			for(ui16 idx = 0; idx < 256; idx++){
				if(box.getBit(*(BitBox_16x16::Address *)&idx)){
					_asm int 3;
					return false;
				}
			}
			box.down();
		}

		//Check basic set/reset/get
		{
			box.up();
			for(ui16 idx = 0; idx < 256; idx++){
				box.setBit(*(BitBox_16x16::Address *)&idx);
				if(!box.getBit(*(BitBox_16x16::Address *)&idx)){
					_asm int 3;
					return false;
				}
				box.resetBit(*(BitBox_16x16::Address *)&idx);
				if(box.getBit(*(BitBox_16x16::Address *)&idx)){
					_asm int 3;
					return false;
				}
			}
			box.down();
		}

		//Check if the count is ok and multiple redundant calls are ok
		{
			box.up();
			for(ui16 idx = 0; idx < 2048; idx++){
				ui8 id = (ui8)idx;
				if((id%2) == 0){
					box.setBit(*(BitBox_16x16::Address *)&id);
				}				
			}
			if(box.getSetCount() != 128){
				_asm int 3;
				return false;
			}
			for(ui8 iByte = 0; iByte < sizeof(box.data); iByte++){
				if(box.data[iByte] != 0x55){
					_asm int 3;
					return false;
				}
			}
			for(ui16 idx = 0; idx < 2048; idx++){
				ui8 id = (ui8)idx;
				if((id%2) == 0){
					box.resetBit(*(BitBox_16x16::Address *)&id);
				}				
			}
			if(box.getSetCount() != 0){
				_asm int 3;
				return false;
			}
			for(ui8 iByte = 0; iByte < sizeof(box.data); iByte++){
				if(box.data[iByte] != 0){
					_asm int 3;
					return false;
				}
			}
			box.down();
		}
		
		return true;
	};
};




/*
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

void * functionTest(void * p){

	WindowsConsoleOutputStream * pConsole = (WindowsConsoleOutputStream *) p;
	pConsole->output("Inside functionTest\n");
	return p;
}
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
	public:
		kq::core::memory::Pointer<DataNode> m_pNext;
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
				pDataNodeOut = pDataNode->m_pData
				m_pTail = pDataNode->m_pNext;
				return true;
			}
			return false;
		};
	};

	kq::core::memory::Pointer<Viewer> createViewer(){
		kq::core::memory::Pointer<Viewer> pRet = kq_core_memory_workerRefCountedClassNew(mem, Viewer);
		pRet->m_pTail = getTail();
		return pRet;
	};

};


struct sz{
};


int main(int /*argc*/, char **){

	{
		kq::core::memory::MemoryWorker mem0;
		kq::core::memory::StandardLibraryMemoryAllocator a;
		a.getMemoryWorker(mem0);

		kq::core::memory::Pointer<kq::core::memory::PooledMemoryAllocator> pPooledAllocator = kq_core_memory_workerRefCountedClassNew(mem0, kq::core::memory::PooledMemoryAllocator, mem0);		
		kq::core::memory::MemoryWorker mem;
		pPooledAllocator->getMemoryWorker(mem);

			
		kq::core::memory::Pointer<DataQueue<sz>> pQ = kq_core_memory_workerRefCountedClassNew(mem, DataQueue<sz>, (mem));
		kq::core::memory::Pointer<DataQueue<sz>::Viewer> pViewer = pQ->createViewer();

		{
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
		}
		//q.createViewer();
		{
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
			pQ->push(kq_core_memory_workerRefCountedBasicNew(mem, sz));
		}

	}

	BitBox_16x16::test();
	
	
	WinMain(0,0,0,0);

}

#include "setjmp.h"


#pragma warning(push)
#pragma warning(disable:4731)

void * remoteCall(void * pStackTop, void * (*pfnFunction)(void *), void * pData){

	void * pRet = 0;

	_asm{
		pusha;
		push ebp;

		mov ebx, pStackTop;

		;push esp
		sub ebx, 4;
		mov [ebx], esp;

		;push pData
		sub ebx, 4;
		mov ecx, pData;
		mov [ebx], ecx;

		;push pfnFunction
		sub ebx, 4;
		mov ecx, pfnFunction;
		mov [ebx], ecx;

		;Set to target context;
		mov esp, ebx;
		mov ebp, 0;

		;pop pfnFunction and call it
		pop ebx;
		call ebx;

		;pop pData
		pop ebx;
	
		;Restore old context;
		pop esp;

		pop ebp;
		mov pRet, eax;
		popa;
	}
	
	return pRet;

};

#pragma warning(pop)


class Texture{
public:

	struct Pixel_RGBA{
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte a;
	};

	class Buffer{
	public:
		GLint width;
		GLint height;
		Pixel_RGBA * address;
		kq::core::memory::MemoryWorker mem;

		Buffer(kq::core::memory::MemoryWorker worker, GLint w, GLint h):mem(worker), width(w), height(h){		
			address = (Pixel_RGBA *)mem(0, sizeof(address[0]) * width * height);
		}

		~Buffer(){
			mem(address, 0);
		}
	};

	kq::core::memory::Pointer<Buffer> m_pBuffer;

	GLuint m_texCam;

	Texture(){
		glGenTextures(1, &m_texCam);
		glBindTexture(GL_TEXTURE_2D, m_texCam);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_videoBuffer.width, m_videoBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_videoBuffer.address);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	}

	~Texture(){
		glDeleteTextures(1, &m_texCam);
		m_texCam = 0;
	}

	void move(double dTime){
		dTime;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pBuffer->width, m_pBuffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pBuffer->address);
	}

	void render(){
		glClear(GL_DEPTH_BUFFER_BIT);

		float fSize = 1.0;
		float fAspect = 1 / 1280.0 * 720.0;
		glBegin(GL_QUADS);

		glColor4d(1, 1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, m_texCam);
		glTexCoord2d(1, 0); glVertex3d(-fSize * fAspect, fSize, 1.0);
		glTexCoord2d(0, 0); glVertex3d(fSize*fAspect, fSize, 1.0);
		glTexCoord2d(0, 1); glVertex3d(fSize*fAspect, -fSize, 1.0);
		glTexCoord2d(1, 1); glVertex3d(-fSize*fAspect, -fSize, 1.0);


		glEnd();


	}
};


class SineTable{


	kq::core::memory::Pointer<float> ptr;
	kq::core::ui32 m_nVals;
	float * m_pVals;

	static const float pi;
	static const float twopi;
	static const float pibytwo;

public:
	SineTable(){
		m_nVals = 0;
		m_pVals = 0;
	}

	void build(kq::core::memory::MemoryWorker & mem, kq::core::ui32 nVals){
		ptr = 0;
		m_pVals = 0;
		m_nVals = 0;

		ptr = kq_core_memory_workerRefCountedMalloc(mem, sizeof(float) * nVals);
		if(ptr){
			m_nVals = nVals;
			m_pVals = ptr.operator ->();

			kq::core::ui32 i = 0;

			while(i < m_nVals){
				float di = twopi * i / m_nVals;
				m_pVals[i] = ::sin(di);
				i++;
			}
		}
	}

	float sin(float d){
		return m_pVals[((kq::core::ui32)(d * m_nVals + 0.5))%m_nVals];
	}

	float cos(float d){
		return sin(d + pibytwo);
	}
};

const float SineTable::pi = 3.14159265358979323846f;
const float SineTable::twopi = 2.0f * pi;
const float SineTable::pibytwo = pi/2.0f;

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/){

	kq::core::memory::MemoryWorker mem0;
	kq::core::memory::StandardLibraryMemoryAllocator a;
	a.getMemoryWorker(mem0);

	kq::core::memory::Pointer<kq::core::memory::PooledMemoryAllocator> pPooledAllocator = kq_core_memory_workerRefCountedClassNew(mem0, kq::core::memory::PooledMemoryAllocator, mem0);
	
	kq::core::memory::MemoryWorker mem;
	pPooledAllocator->getMemoryWorker(mem);




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

	kq::core::ui32 nBytes = 1024 * 10;
	char * pStack = (char *)malloc(nBytes);
	if(remoteCall(pStack + nBytes, functionTest, pConsole) == pConsole){
		pConsole->output("Remote Call Test Successfull\n");
	}
	

	kq::core::memory::Pointer<ITicker> pTicker = kq_core_memory_workerRefCountedClassNew(mem, WindowsAPITicker);
	kq::core::memory::Pointer<IStopWatch> pStopWatch = kq_core_memory_workerRefCountedClassNew(mem, StopWatch, pTicker);
	kq::core::memory::Pointer<IClock> pClock(kq_core_memory_workerRefCountedClassNew(mem, Clock, 80.0, kq_core_memory_workerRefCountedClassNew(mem, StopWatch, pTicker), kq_core_memory_workerRefCountedClassNew(mem, WindowsAPISleeper)));


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

								
								glEnable(GL_TEXTURE_2D);
								Texture t;
								t.m_pBuffer = kq_core_memory_workerRefCountedClassNew(mem, Texture::Buffer, mem, dmCurrent.dmPelsWidth, dmCurrent.dmPelsHeight);
								
								
								double dTime = 0.0;
								pStopWatch->reset();

								SineTable sinTable;
								sinTable.build(mem, 256);


								LARGE_INTEGER iCountOld;
								iCountOld.QuadPart = 0;
								while(bContinue){

									//Process Local Messages
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

									
									//Move the world
									{																														
										double dSecondsSinceLastReset = (double)pStopWatch->getTickCount() / (double)pStopWatch->getTicksPerSecond();
										pStopWatch->reset();
										dTime += dSecondsSinceLastReset;

										
										
										{
											kq::core::memory::Pointer<Texture::Buffer> pBuffer = t.m_pBuffer;
											Texture::Pixel_RGBA * first = pBuffer->address;
											Texture::Pixel_RGBA * current = first;

											GLint i, n, w,h, x, y;
											//for(x = 0; x < pBuffer->width; x++){
											//	for(y = 0; y < pBuffer->height; y++){
											{
												i = 0;
												w = pBuffer->width;
												h = pBuffer->height;
												n = w * h;

												double t = 1 + sinTable.sin(0.75 + dTime * 0.1 * dTime * 0.1);
												t *= 0.5;
												


												//memset(first, 0, sizeof(*first) * n);
												while(i < n)
												{
													x = i % w;
													y = i / w;

													//float s = (sinTable.sin(0.75 + 1.0 * x / w * y / h + t) + 1) * 0.5;s;
													//float c = (sinTable.cos(0.75 + 1.0 * x / w * y / h - t) + 1) * 0.5;c;
													
													current->r = (GLubyte)(255.0f * (w-x) / w * (h-y) / h * t + 0.5);
													current->g = (GLubyte)(255.0f * (w-x) / w * y / h * t + 0.5);
													current->b = (GLubyte)(255.0f * x / w * y / h * t + 0.5);
													current->a = 0;

													current++;
													i++;


												}												
											}

										}

										t.move(dSecondsSinceLastReset);
									}

									//Render the world
									{
										t.render();
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
