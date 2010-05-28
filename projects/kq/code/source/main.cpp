
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
/*
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

*/


#include <new>

#define kq_core_memory_workerNew(memworker, classname, ...) (new (memworker(0, sizeof(classname))) classname __VA_ARGS__)
#define kq_core_memory_workerDelete(memworker, classname, obj) (obj->~classname());(memworker(obj, 0))



#define kq_core_memory_workerEasyClassTypeRefCountedNew(memworker, classname, ...) (kq_core_memory_workerNew(memworker, EasyClassTypeRefCounter<classname>, (memworker, kq_core_memory_workerNew(memworker, classname, __VA_ARGS__), 0, EasyClassTypeRefCounter<classname>::AtLast) ))
#define kq_core_memory_workerEasyBasicTypeRefCountedNew(memworker, classname, ...) (kq_core_memory_workerNew(memworker, EasyBasicTypeRefCounter<classname>, (memworker, kq_core_memory_workerNew(memworker, classname, __VA_ARGS__), 0, EasyBasicTypeRefCounter<classname>::AtLast) ))

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

namespace kq{
	namespace flows{
		namespace konnect{


			typedef kq::core::ui64 ID;

			class IStreamReader{
			public:
				virtual void readOpen(ID stream) = 0;
				virtual bool readData(ID stream, ID & data) = 0;
				virtual void readClose(ID stream) = 0;
			};

			class IStreamWriter{
			public:
				virtual void writeOpen(ID stream) = 0;
				virtual void writeData(ID stream, ID data) = 0;
				virtual void writeClose(ID stream) = 0;								
			};

			class INetwork;
			
			class IWorker{				
			public:
				virtual bool up(INetwork * pNetwork, IStreamWriter * pWriter) = 0;
				virtual bool process(INetwork * pNetwork, IStreamWriter  * pWriter, IStreamReader * pReader) = 0;				
				virtual bool down(INetwork * pNetwork, IStreamWriter * pWriter) = 0;
			};

			class INetwork{				
				virtual bool addNode(kq::core::memory::Pointer<IWorker> pWorker) = 0;
				virtual bool removeNode(kq::core::memory::Pointer<IWorker> pWorker) = 0;
				virtual kq::core::ui64 rest(kq::core::ui64 nMicroSeconds);
			};
		};
	};
};


class Network:public kq::flows::konnect::INetwork{

	struct NodeInfo{
	public:
		kq::core::memory::Pointer<kq::flows::konnect::IWorker> pWorker;	
	};
	

	static const ui32 nMaxNodes = 16;
	NodeInfo info[nMaxNodes];
	ui32 iNextEmptyNodeSlot;

	kq::core::ui64 sleep(kq::core::ui64 nMicroSeconds){
		return nMicroSeconds;
	};

	bool addNode(kq::core::memory::Pointer<kq::flows::konnect::IWorker> pWorker){
		bool bRet = false;
		if(iNextEmptyNodeSlot < nMaxNodes){
			info[iNextEmptyNodeSlot].pWorker = 0;pWorker;
			iNextEmptyNodeSlot++;
			bRet= true;
		}
		return bRet;
	};

	bool removeNode(kq::core::memory::Pointer<kq::flows::konnect::IWorker> pWorker){
		ui32 i;
		bool bRet = false;
		for(i = 0; i < iNextEmptyNodeSlot; i++){
			if(info[i].pWorker == pWorker){
				info[i].pWorker = 0;
				bRet = true;
				break;
			}
		}

		return bRet;
	};
};


class TestWorker:public kq::flows::konnect::IWorker{
public:
	bool up(kq::flows::konnect::INetwork * /*pNetwork*/, kq::flows::konnect::IStreamWriter * pWriter){
		pWriter->writeOpen(0);
	}

	bool process(kq::flows::konnect::INetwork * /*pNetwork*/, kq::flows::konnect::IStreamWriter * pWriter, kq::flows::konnect::IStreamReader * /*pReader*/){
		bool bTrue = true;
		while(bTrue){
			pWriter->writeData(0, 0);
		}
	}

	bool down(kq::flows::konnect::INetwork * /*pNetwork*/, kq::flows::konnect::IStreamWriter * pWriter){
		pWriter->writeClose(0);
	}
};

/*
namespace kq{
	namespace flows{
		namespace konnect{

			kq::core::memory::Pointer<Network> m_pTest;
		};
	};
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
		return kq_core_memory_workerRefCountedNew(mem, LinkNode);
	};

	kq::core::memory::Pointer<DataNode> createDataNode(){
		return kq_core_memory_workerRefCountedNew(mem, DataNode);
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
		kq::core::memory::Pointer<LinkNode> m_pNewTail = kq_core_memory_workerEasyClassTypeRefCountedNew(mem, LinkNode);
		kq::core::memory::Pointer<DataNode> m_pNewTailData = kq_core_memory_workerEasyClassTypeRefCountedNew(mem, DataNode);

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
		kq::core::memory::Pointer<Viewer> pRet = kq_core_memory_workerEasyClassTypeRefCountedNew(mem, Viewer);
		pRet->m_pTail = getTail();
		return pRet;
	};

};

struct sz{
	int arr[64];
};

int main(int /*argc*/, char **){

	BitBox_16x16::test();

	kq::core::memory::MemoryWorker mem0;
	kq::core::memory::StandardLibraryMemoryAllocator a;
	a.getMemoryWorker(mem0);

	kq::core::memory::MemoryWorker mem;
	kq::core::memory::PooledMemoryAllocator b(mem0);
	b.getMemoryWorker(mem);

	/*
	kq::core::memory::Pointer<DataQueue<sz>> pQ = kq_core_memory_workerEasyClassTypeRefCountedNew(mem, DataQueue<sz>, (mem));
	kq::core::memory::Pointer<DataQueue<sz>::Viewer> pViewer = pQ->createViewer();

	{
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
	}
	//q.createViewer();
	{
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
		pQ->push(kq_core_memory_workerEasyBasicTypeRefCountedNew(mem, sz));
	}

	*/

	//mem(0, 100);

	//WinMain(0,0,0,0);

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

//
//int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/){
//
//
//	if(!GetStdHandle(STD_OUTPUT_HANDLE)){
//		if(AttachConsole(ATTACH_PARENT_PROCESS) == FALSE){
//			if(AllocConsole() == FALSE){
//				return -1;
//				
//			}
//		}	
//	}
//
//	WindowsConsoleOutputStream console;
//	WindowsConsoleOutputStream * pConsole = &console;
//	pConsole->output("Console Init...\tAttached.\n");
//
//	kq::core::ui32 nBytes = 1024 * 10;
//	char * pStack = (char *)malloc(nBytes);
//	if(remoteCall(pStack + nBytes, functionTest, pConsole) == pConsole){
//		pConsole->output("Remote Call Test Successfull\n");
//	}
//	
//
//	kq::core::memory::Pointer<ITicker> pTicker = new kq::core::memory::RefCounter(new WindowsAPITicker());
//	kq::core::memory::Pointer<IStopWatch> pStopWatch = new kq::core::memory::RefCounter(new StopWatch( pTicker ));
//	kq::core::memory::Pointer<IClock> pClock(new kq::core::memory::RefCounter(new Clock(80, new kq::core::memory::RefCounter(new StopWatch( pTicker )), new kq::core::memory::RefCounter(new WindowsAPISleeper))));
//
//
//	DISPLAY_DEVICE dd = {0};
//	dd.cb = sizeof(dd);
//	DEVMODE dm = {0};
//	dm.dmSize = sizeof(dm);
//
//	DEVMODE dmCurrent = {0};
//	dmCurrent.dmSize = sizeof(dmCurrent);
//
//	int iDisplayDevice = 0;
//
//	
//	DWORD dwCheck = DISPLAY_DEVICE_ACTIVE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP | DISPLAY_DEVICE_PRIMARY_DEVICE;	
//	
//	bool bRequestedResolutionSet = false;
//
//	while( !bRequestedResolutionSet && EnumDisplayDevices(0, iDisplayDevice, &dd, 0) ){
//
//		EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dmCurrent);
//		
//		//Search of displays which are relevant
//		if((dd.StateFlags & dwCheck) == dwCheck){
//
//			pConsole->output(L"Using Display...\t%s.\n", dd.DeviceString);			
//
//			int iDevmode = 0;
//			while(EnumDisplaySettings(dd.DeviceName, iDevmode, &dm)){
//				
//				if(dm.dmBitsPerPel >= 32){
//					if(
//						dm.dmPelsWidth == dmCurrent.dmPelsWidth
//						&& dm.dmPelsHeight == dmCurrent.dmPelsHeight
//						&& dm.dmBitsPerPel == dmCurrent.dmBitsPerPel
//						&& dm.dmDisplayFrequency == dmCurrent.dmDisplayFrequency
//					){
//
//						//pConsole->output(L"***", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel, dm.dmDisplayFrequency);
//						pConsole->output(L"Using Current Resolution...\t%4dx%4d %2dbits @ %2dHz\n", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel, dm.dmDisplayFrequency);					
//						pConsole->output("Switching To FullScreen...\t");
//						if((ChangeDisplaySettingsEx(dd.DeviceName, &dmCurrent, 0, CDS_FULLSCREEN, 0) == DISP_CHANGE_SUCCESSFUL)){
//							bRequestedResolutionSet = true;
//							pConsole->output("Done.\n");
//							break;
//						}else{
//							pConsole->output("Failed!\n");
//						}
//					}										
//				}
//				iDevmode++;
//			}
//		}
//		iDisplayDevice++;
//	}			
//	
//
//	if(bRequestedResolutionSet){
//		WNDCLASSEX wc;
//		wc.cbSize = sizeof(wc);
//		wc.style = CS_OWNDC;
//		wc.lpfnWndProc = wndproc;
//		wc.cbClsExtra = 0;
//		wc.cbWndExtra = 0;
//		wc.hInstance = hInstance;
//		wc.hIcon = 0;
//		wc.hCursor = LoadCursor(0, IDC_APPSTARTING);
//		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
//		wc.lpszMenuName = 0;
//		wc.lpszClassName = L"KQWindowClass";
//		wc.hIconSm = 0;
//
//
//		pConsole->output(L"Registering Class For main window(%s)...\t", wc.lpszClassName);
//		ATOM regResult = RegisterClassEx(&wc);
//		if(regResult){
//
//
//			LPWSTR lpszWindowName = L"Game Window";
//			pConsole->output(L"%d\nCreating Window(%s::%s)...", regResult, wc.lpszClassName, lpszWindowName);
//
//			HWND hMainWindow = CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, lpszWindowName, WS_POPUP, 0, 0, dmCurrent.dmPelsWidth, dmCurrent.dmPelsHeight, 0, 0, hInstance, 0);
//			if(hMainWindow){
//				pConsole->output("%x\nGetting Device Context...\t", hMainWindow);
//				HDC hMainWindowDC = GetDC(hMainWindow);
//				if(hMainWindowDC){
//					pConsole->output("%x\n", hMainWindowDC);
//					PIXELFORMATDESCRIPTOR pfd = {0};
//					pfd.nSize = sizeof(pfd);
//					pfd.nVersion = 1;
//					pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
//					pfd.iPixelType = PFD_TYPE_RGBA;
//					pfd.cColorBits = 24;
//					pfd.cDepthBits = 16;
//					pfd.iLayerType = PFD_MAIN_PLANE;
//
//					pConsole->output("Trying to set pixel format...\t");
//
//					int iPixelFormat = ChoosePixelFormat(hMainWindowDC, &pfd);
//					if(iPixelFormat && (SetPixelFormat(hMainWindowDC, iPixelFormat, &pfd) == TRUE)){
//						pConsole->output("Set\nCreating OpenGL Context...\t");
//						HGLRC hGL = wglCreateContext(hMainWindowDC);
//						if(hGL){
//							pConsole->output("%x\n", hGL);
//							HGLRC hGLOld = wglGetCurrentContext();
//							if((wglMakeCurrent(hMainWindowDC, hGL) == TRUE)){
//								MSG msg;
//								bool bContinue = true;
//								pConsole->output("Showing Main Window...\nEntering Game loop...\n");
//
//								ShowWindow(hMainWindow, SW_MAXIMIZE);
//
//								kq::core::memory::Pointer<SceneGraphNode> pRoot;
//
//								LARGE_INTEGER iCountOld;
//								iCountOld.QuadPart = 0;
//								while(bContinue){
//									BOOL bMsg = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
//									if(bMsg == TRUE){
//										switch(msg.message){
//											case WM_QUIT:
//												pConsole->output("Got Quit Message...\n");
//												bContinue = false;					
//												break;
//											default:
//												TranslateMessage(&msg);
//												DispatchMessage(&msg);
//												break;
//										};
//
//										//Go back and process all messages in queue before proceeding
//										continue;
//									}
//
//									
//									{
//
//										
//
//										if(!pRoot){
//											pRoot = (new kq::core::memory::RefCounter (new SceneGraphNode_Root));
//
//											kq::core::memory::Pointer<SceneGraphNode_OpenGL_First> pOpenGL = new kq::core::memory::RefCounter ( new SceneGraphNode_OpenGL_First);
//
//											kq::core::memory::Pointer<SceneGraphNode_OpenGL_Model> pModel = new kq::core::memory::RefCounter (new SceneGraphNode_OpenGL_Model);
//											pModel->pModel = new kq::core::memory::RefCounter ( new Model);
//											pModel->pModel->tabF = TriangleModel.tabF;
//											pModel->pModel->tabV = TriangleModel.tabV;
//
//											kq::core::memory::Pointer<SceneGraphNode_OpenGL_Transform> pTransform = new kq::core::memory::RefCounter (new SceneGraphNode_OpenGL_Transform);
//											
//
//											kq::core::memory::Pointer<SceneGraphNode_OpenGL_Model> pModel2 = new kq::core::memory::RefCounter (new SceneGraphNode_OpenGL_Model);
//											pModel2->pModel = new kq::core::memory::RefCounter ( new Model);
//											pModel2->pModel->tabF = TriangleModel.tabF;
//											pModel2->pModel->tabV = TriangleModel.tabV;
//
//											pTransform->m_pFirstChild = pModel2.castStatic<SceneGraphNode>();
//
//											pOpenGL->m_pFirstChild = pTransform.castStatic<SceneGraphNode>();
//											pTransform->m_pNextSibling = pModel.castStatic<SceneGraphNode>();
//
//
//											pRoot->m_pFirstChild = pOpenGL.castStatic<SceneGraphNode>();
//
//											pStopWatch->reset();
//											
//										}
//
//										
//										double dSecondsSinceLastReset = (double)pStopWatch->getTickCount() / (double)pStopWatch->getTicksPerSecond();
//										pStopWatch->reset();
//
//										pRoot->operate(SceneGraphNode::otMove, dSecondsSinceLastReset);
//										pRoot->operate(SceneGraphNode::otRender);
//
//
//										
//									}
//
//									SwapBuffers(hMainWindowDC);
//									pClock->waitTillNextCycle();								
//								}
//
//								wglMakeCurrent(hMainWindowDC, hGLOld);
//							}else{
//								pConsole->output("Failed to set current context");
//							}
//							wglDeleteContext(hGL);
//						}else{
//							pConsole->output("Failed to create OpenGl Context");
//						}
//
//					}else{
//						pConsole->output("Failed to fix pixel format\n");
//					}
//
//					ReleaseDC(hMainWindow, hMainWindowDC);
//				}else{
//					pConsole->output("Failed to get DC\n");
//				}
//
//				DestroyWindow(hMainWindow);
//			}else{
//				pConsole->output("Failed to Create window\n");
//			}
//
//			UnregisterClass(wc.lpszClassName, hInstance);
//		}else{
//			pConsole->output("Failed to Register Class\n");
//		}
//	}else{
//		pConsole->output("Couldnot Set Requested Resolution\n");
//	}
//
//	pConsole->output("Restoring Default Display Settings\n");
//	ChangeDisplaySettingsEx(0, 0, 0, 0, 0);
//
//	pConsole->output("Detaching Console\n");
//	FreeConsole();
//	return 0;
//}
