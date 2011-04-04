#include "core.hpp"

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
/*
static int iDepth = 0;

class LogInOut{
	const char * m_pFunction;
	void log(bool bEntry){
		const char * sMark;
		if(bEntry){
			sMark = "{";
		}
		else{
			sMark = "}";
		}
		char * tabs;
		tabs = new char[iDepth + 1];
		memset(tabs, '\t', sizeof(char) * iDepth);
		tabs[iDepth] = 0;

		if(bEntry){
			printf("%s%s%s\n", tabs, m_pFunction, sMark);
		}else{
			printf("%s%s\n", tabs, sMark);
		}
		delete [] tabs;
	}
public:

	void pushdepth(){
		char * tabs;
		tabs = new char[iDepth + 1];
		memset(tabs, '\t', sizeof(char) * iDepth);
		tabs[iDepth] = 0;
		printf("%s", tabs);
		delete [] tabs;
	}

	LogInOut(const char * pFunction):m_pFunction(pFunction){

		log(true);
		iDepth++;
	}
	~LogInOut(){
		iDepth--;
		log(false);
	}
};

#define LOGINOUT LogInOut var##__FUNCTION__(__FUNCTION__);
#define LOGDEPTH var##__FUNCTION__.pushdepth()
*/
/*
namespace kq{

	namespace flows{


		class IResource{
		public:
			//The locks can be used as follows
			virtual bool lockResource(bool bExclusive) = 0;
			virtual bool unlockResource() = 0;
		};

		class IFlowWriter: public IResource{
			//When a writer is exclusively locked by a processor it means that only that processor can write to this flow.
			//When a writer is shared locked by a processor multiple processors can atomically write to the flow
		public:
			virtual void write(const void * pBytes, kq::core::ui64 nBytes);
		};

		class IFlowReader: public IResource{
		public:
			virtual void read(void * pBytes, kq::core::ui64 nBytes);
		};

		class IFlowTuple: public IResource{
			//An ordered set of flows
		};

		class IFlowWriterTuple{
		public:
			virtual kq::core::memory::Pointer<IFlowWriter> getFlowWriter(kq::core::ui64 iFlow) = 0;
		};

		class IFlowReaderTuple{
		public:
			virtual kq::core::memory::Pointer<IFlowReader> getFlowReader(kq::core::ui64 iFlow) = 0;
			virtual void readRecord(void ** pBytes, kq::core::ui64 * nBytes) = 0;
		};


		class IFlowInterfaceDescriptor{

		public:
			virtual void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs, kq::core::ui64 * pOutputs) = 0;
			//e.g. 2, 1
		};

		class IProcessor: public IFlowInterfaceDescriptor{
			//Reads from input streams...processes....writes to output streams
			//Lifecycle: construct...(attach, work, detach)^n....desctruct
		public:

			//Sets the output and input streams, acquire locks if required, if not locked queues might be shared.
			virtual void attachFlows(kq::core::memory::Pointer<IFlowWriterTuple> inputs, kq::core::memory::Pointer<IFlowReaderTuple> outputs) = 0;

			//There are changes that signals/setjumps/longjumps may switch the OS level thread without telling this object
			//It is recommended to not do thread local stuff (TLS, etc)
			//Once doWork returns..the object may be destroyed or attach may be called
			virtual void doWork() = 0;

			//Detaches the input output streams...remove any references held on streams
			virtual void detachFlows() = 0;
		};

		class IProcess:public IFlowInterfaceDescriptor{
			//Represents a process and creates IProcessor objects which actually process streams of data
			//e.g. "The process of merging 2 streams into one

		public:

			virtual kq::core::memory::Pointer<IProcessor> generateProcessor() = 0;
			//Create a IWorker object
		};


		class IFlow : public IResource{
			//Fifo
		public:
			//User has forgotten this flow object and hence has no way of getting back to this one
			//Which means there can be no more readers attaching to this one...so start forgetting read data
			void disownedByUser();
		};





		class Machine;
		class Value{

			Machine * m_pMachine;
			kq::core::memory::Pointer<IProcessor> m_pProcessor;

			friend class Machine;
			Value(Machine * pMachine){
				LOGINOUT;
				m_pMachine = pMachine;
			}

			void setProcessorValue(kq::core::memory::Pointer<IProcessor> pProcessor){
				LOGINOUT;
				m_pProcessor = pProcessor;
			}


		public:
			//Once this is called this value can no longer be used by the user
			//This means we can start deleting any stored data on the attached flow.
			~Value(){LOGINOUT;}
		};



		typedef kq::core::memory::Pointer<Value> Variable;


		class Machine{


			kq::core::memory::MemoryWorker mem;

			struct EdgeNode{
				Variable pSrc;
				Variable pDest;

				kq::core::memory::Pointer<EdgeNode> pNext;
			};

			typedef kq::core::memory::Pointer<EdgeNode> PEdgeNode;
			PEdgeNode m_pRoot;

		public:

			Machine(kq::core::memory::MemoryWorker & memory):mem(memory){
				LOGINOUT;
			}

			~Machine(){
				LOGINOUT;
			}

			bool _confirmFlow(Variable pSrc, Variable pDest){
				LOGINOUT;
				kq::core::memory::Pointer<IProcessor> pSrcProcessor = pSrc->m_pProcessor;
				kq::core::memory::Pointer<IProcessor> pDestProcessor = pDest->m_pProcessor;
				kq::core::ui64 out, in;
				pSrcProcessor->getNumberOfInputOutputStreams(0, &out);
				pDestProcessor->getNumberOfInputOutputStreams(&in, 0);
				if(in == out){
					PEdgeNode pNewRoot(kq_core_memory_workerRefCountedClassNew(mem, EdgeNode));
					pNewRoot->pSrc = pSrc;
					pNewRoot->pDest = pDest;
					pNewRoot->pNext = m_pRoot;
					m_pRoot = pNewRoot;
					return true;
				}


				return false;
			}

			static bool confirmFlow(Variable pSrc, Variable pDest){
				LOGINOUT;
				return (pSrc && pDest && pSrc->m_pMachine && (pSrc->m_pMachine == pDest->m_pMachine) && pSrc->m_pMachine->_confirmFlow(pSrc, pDest));
			}

			Variable variableFromProcessor(kq::core::memory::Pointer<IProcessor> pProcessor){
				LOGINOUT;
				kq::core::memory::Pointer<Value> pRet(kq_core_memory_workerRefCountedClassNew(mem, Value, this));
				pRet->setProcessorValue(pProcessor);
				return pRet;
			}

		};


		void operator >> (Variable a, Variable b){
			LOGINOUT;
			Machine::confirmFlow(a, b);
		};
	}
}

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;


using namespace kq::flows;


class Input: public IProcessor{
	Pointer<IFlowWriter> w;
public:
	void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs,kq::core::ui64 * pOutputs){
		if(pInputs)*pInputs = 0;
		if(pOutputs)*pOutputs = 1;
	}

	void attachFlows(Pointer<IFlowWriterTuple> tupleW, Pointer<kq::flows::IFlowReaderTuple> tupleR){
		w = tupleW->getFlowWriter(0);
	}

	void doWork(void){
		bool bTrue = true;
		while(bTrue){
			ui32 val = (ui32)rand();
			w->write(&val, sizeof(val));
			printf("Writer wrote %d", (int)val);
		}
	}

	void detachFlows(void){
		w = 0;
	}
};


class Output: public IProcessor{
	Pointer<IFlowReader> r;
public:
	void getNumberOfInputOutputStreams(kq::core::ui64 * pInputs,kq::core::ui64 * pOutputs){
		if(pInputs)*pInputs = 1;
		if(pOutputs)*pOutputs = 0;
	}

	void attachFlows(Pointer<IFlowWriterTuple> tupleW, Pointer<kq::flows::IFlowReaderTuple> tupleR){
		r = tupleR->getFlowReader(0);
	}

	void doWork(void){
		ui32 n,i;
		n = 10;
		i = 0;
		while(i < n){
			ui32 val;
			r->read(&val, sizeof(val));
			printf("Reader got %d", (int) val);
			i++;
		};
	}

	void detachFlows(void){
		r = 0;
	}
};


class Test{

public:

	int a;
	Test(){printf("%p", this);LOGINOUT;}
	~Test(){printf("%p", this);LOGINOUT;}
};
*/

/*

class TreeMap{

public:
	struct Node{

		Node *** children;
		Node * parent;
		kq::core::ui8 parentIndex;
	};



protected:

	//Start(Allocation, Deallocation and UserData)
	kq::core::memory::MemoryWorker mem;
	kq::core::ui16 m_nUserBytes;
	Node * allocateNode(void){return (Node *)mem(0, sizeof(Node) + m_nUserBytes);}
	void deallocateNode(Node * p){mem(p, 0);}
	kq::core::ui8 * nodeToUserData(Node * p){return ((kq::core::ui8 *)p+sizeof(Node));}
	Node * userDataToNode(kq::core::ui8 * p){return (Node *)(p - sizeof(Node));}
	//End(Allocation, Deallocation and UserData)

	Node * m_pRoot;

	void traversePath(kq::core::ui8 * pKeyStart, kq::core::ui8 ** ppKeyEnd, Node ** ppStart){

		if(!*ppStart){
			return;
		}

		Node * & pCurr = *ppStart;//Allocation, Deallocation and UserData
		Node * pNext;

		kq::core::ui8 * & pKeyEnd = *ppKeyEnd;

		while(pKeyEnd >= pKeyStart && pCurr->children && ((pNext = pCurr->children[*pKeyEnd])!= 0) ){
			pKeyEnd--;
			pCurr = pNext;
		}
	}

	void createPath(kq::core::ui8 * pKeyStart, kq::core::ui8 ** ppKeyEnd, Node ** ppStart){

		kq::core::ui8 * &pKeyEnd = *ppKeyEnd;
		Node * & pCurr = *ppStart;

		Node ** pTempNodes = 0;
		const kq::core::ui32 nTempNodes = pKeyEnd - pKeyStart + 1 + ((pCurr)?0:1);
		{

			kq::core::ui32 iNode;
			pTempNodes = (Node **)mem(0, sizeof(Node *) * nTempNodes);
			for(iNode = 0; iNode < nTempNodes; iNode++){
				pTempNodes[iNode] = allocateNode();
				if(!pTempNodes[iNode]){
					break;
				}
			}

			if(iNode != nTempNodes){
				iNode--;
				while(iNode >= 0){
					deallocateNode(pTempNodes[iNode]);
					iNode--;
				}
				mem(pTempNodes,0);
				return;
			}
		}

		kq::core::ui32 iTempNode = 0;

		if(!pCurr){
			pCurr = pTempNodes[iTempNode++];
			pCurr->parent = 0;
			pCurr->parentIndex = 0;
			memset(pCurr->children, 0, sizeof(pCurr->children));
			if(m_pRoot == 0){
				m_pRoot = pCurr;
			}
		}

		kq::core::ui8 index;
		Node * pNext;
		while(pKeyEnd >= pKeyStart){
			pNext = pTempNodes[iTempNode++];

			index = *pKeyEnd;
			pCurr->children[index] = pNext;
			pNext->parent = pCurr;
			pNext->parentIndex = index;
			memset(pNext->children, 0, sizeof(pCurr->children));
			pCurr = pNext;
			pKeyEnd--;
		}
		mem(pTempNodes, 0);
	}


public:
	TreeMap(kq::core::memory::MemoryWorker & memworker, kq::core::ui8 nUserBytes):mem(memworker), m_pRoot(0), m_nUserBytes(nUserBytes){

	}

	kq::core::ui8 * findUserData(kq::core::ui8 * pKeyStart, kq::core::ui8 nBytes){
		kq::core::ui8 * pRet = 0;
		kq::core::ui8 * pKeyEnd = pKeyStart + nBytes - 1;
		Node * pNode = m_pRoot;
		traversePath(pKeyStart, &pKeyEnd, &pNode);
		if((pKeyEnd < pKeyStart)){
			pRet = nodeToUserData(pNode);
		}
		return pRet;
	}

	kq::core::ui8 * findOrCreateUserData(kq::core::ui8 * pKeyStart, kq::core::ui8 nBytes){
		kq::core::ui8 * pRet = 0;
		kq::core::ui8 * pKeyEnd = pKeyStart + nBytes - 1;
		Node * pNode = m_pRoot;
		traversePath(pKeyStart, &pKeyEnd, &pNode);
		if((pKeyEnd < pKeyStart)){
			pRet = nodeToUserData(pNode);
		}
		else{
			createPath(pKeyStart, &pKeyEnd, &pNode);
			if(pKeyEnd < pKeyStart){
				pRet = nodeToUserData(pNode);
			}
		}
		return pRet;
	}

	//void destroyUserData(kq::core::ui8 * pKeyStart, kq::core::ui8 nBytes){
	//	destroyPath(pKeyStart, pKeyStart + nBytes - 1);
	//}

	~TreeMap(){

	}
};

*/

/*

class Links{
public:
	typedef kq::core::ui8 USmall;
	typedef void * PNode;

protected:
	kq::core::memory::MemoryWorker mem;

	const USmall m_nBytesInNodeID;

	const USmall m_nNibblesPerByte;
	const USmall m_nBitsPerNibble;

	//2 NodeIDs are an Address
	const USmall m_nBytesInAddress;
	const USmall m_nNibblesInAddress;
	//Some nodes in a key are segment
	const USmall m_nBytesInSegment;
	const USmall m_nNibblesInSegment;
	//The remaining are offset
	const USmall m_nBytesInOffset;
	const USmall m_nNibblesInOffset;


	USmall * m_pAddressBuffer;
	void interlace(PNode p1, PNode p2);

	kq::core::data::BPlusTree * m_pBitTree;
	kq::core::data::BPlusTree * m_pDataTree;

	struct BitGrid{
		USmall nBitsSet;
		USmall pBits[1];
	};

	BitGrid * m_pGridBuffer;
	void gridCreate();
	void gridDestroy();
	void gridSet();
	void gridReset();


public:
	Links(kq::core::memory::MemoryWorker &memworker, USmall nBytesInNodeID);
	~Links();

	bool link(PNode n1, PNode n2, void ** data = 0, void ** dataOld = 0);
	bool unLink(PNode n1, PNode n2, void ** dataOld = 0);
	bool isLinked(PNode n1, PNode n2, void ** data = 0);


};

using namespace kq::core::data;
Links::Links(kq::core::memory::MemoryWorker &memworker, USmall nBytesInNodeID)
	:mem(memworker),
	m_nBitsPerNibble(4),
	m_nNibblesPerByte(8/m_nBitsPerNibble),

	m_nBytesInNodeID(nBytesInNodeID),
	m_nBytesInAddress(m_nBytesInNodeID * 2),
	m_nNibblesInAddress(m_nBytesInAddress * m_nNibblesPerByte),

	m_nBytesInOffset(1),
	m_nNibblesInOffset(m_nBytesInOffset * m_nNibblesPerByte),

	m_nBytesInSegment(m_nBytesInAddress - m_nBytesInOffset),
	m_nNibblesInSegment(m_nNibblesInAddress - m_nNibblesInOffset)
{
	m_pAddressBuffer = (USmall *)mem(0, m_nBytesInAddress);
	m_pBitTree = kq_core_memory_workerNew(mem, kq::core::data::BPlusTree, (mem, m_nBytesInSegment));
	m_pDataTree = kq_core_memory_workerNew(mem, kq::core::data::BPlusTree, (mem, m_nBytesInAddress));
}

void Links::interlace(PNode p1, PNode p2){
	kq::core::ui8 * p[2] = {(ui8 *)p1, (ui8*)p2};
	USmall iByte = 0;
	while(iByte < m_nBytesInNodeID){
		m_pAddressBuffer[2*iByte] = p[0][iByte];
		m_pAddressBuffer[2*iByte + 1] = p[1][iByte];
		iByte++;
	}
}

bool Links::link(PNode n1, PNode n2, void ** data, void ** dataOld){
	bool bRet = false;
	interlace(n1, n2);

	BPlusTree::Path pthBit(m_pBitTree);
	BitGrid * pGridOld;
	m_pGridBuffer = 0;
	if(pthBit.init_moveTo(m_pAddressBuffer, (void **)&pGridOld)){
		m_pGridBuffer = pGridOld;
	}

	if(!m_pGridBuffer){
		gridCreate();
		if(m_pGridBuffer){
			pthBit.write(m_pGridBuffer);
		}
	}

	if(m_pGridBuffer){
		gridSet();
		if(data){
			m_pDataTree->map(m_pAddressBuffer, *data, dataOld);
		}
		bRet = true;
	}


	return bRet;
}

bool Links::unLink(PNode n1, PNode n2, void ** dataOld){
	bool bRet = false;
	interlace(n1, n2);

	BPlusTree::Path pthBit(m_pBitTree);
	BitGrid * pGridOld;
	m_pGridBuffer = 0;
	if(pthBit.init_moveTo(m_pAddressBuffer, (void **)&pGridOld)){
		m_pGridBuffer = pGridOld;
		gridReset();

		if(!m_pGridBuffer){
			pthBit.write(0);
		}

		bRet = m_pDataTree->map(m_pAddressBuffer, 0, dataOld);
	}

	return bRet;
}

bool Links::isLinked(PNode n1, PNode n2, void ** data){
	bool bRet = false;
	interlace(n1, n2);

	BPlusTree::Path pthBit(m_pBitTree);
	BitGrid * pGridOld;
	m_pGridBuffer = 0;
	if(pthBit.init_moveTo(m_pAddressBuffer, (void **)&pGridOld)){
		bRet = true;
		if(*data){
			m_pDataTree->lookup(m_pAddressBuffer);
		}
	}
	return bRet;
}

void Links::gridCreate(){
	ULarge nBytesInGrid = 1 << ((m_nBytesInOffset >> 3) - 3);
	ULarge nSizeOfGrid = sizeof(BitGrid) + nBytesInGrid - 1;
	m_pGridBuffer = (BitGrid *)mem(0, nSizeOfGrid);
	if(m_pGridBuffer){
		memset(m_pGridBuffer, 0, nSizeOfGrid);
	}
}

void Links::gridDestroy(){
	mem(m_pGridBuffer, 0);
	m_pGridBuffer = 0;
}

void Links::gridSet(){
	ui64 iByte = 0;
	memcpy()
}

void Links::gridReset(){
}
*/

#include "sys/time.h"
//#include "windows.h"


typedef kq::core::ui32 UCount;


class IHead{
	//kq::core::memory::Pointer<ITape> m_pTape;
public:

	UCount move(UCount units, UCount direction){
		units;
		direction;
		return 0;
	}

	bool read(void const ** location, UCount * units){
		*location = 0;
		* units = 0;
		return false;
	};

	bool write(void const * location, UCount units){
		units;
		location;
		return false;
	}

};

#include "ui.hpp"
using namespace kq::ui;

#include "unistd.h"

class A{

	int i;
};
class B: /*virtual*/ public A{

	int j;
};

class C:/*virtual*/ public A{

};

class D:public B, public C{

};

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;

int main(int /*argc*/, char ** /*argv*/){
	//LOGINOUT;

	//Create std allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);


	{
	    MemoryWorker mem = memStd;
		PooledMemoryAllocator allocPool(memStd);
		allocPool.getMemoryWorker(mem);

        //kq::core::data::BPlusTree_test(mem);

		{
			kq::core::memory::Pointer<Window> pWindow;
			kq::core::memory::Pointer<kq::ui::UserInterface> pUI;
			{
				pUI = kq::ui::UserInterface::createInstance(mem);

				if(pUI){

					if(pUI->getScreenCount()){
						Pointer<Screen> pScreen = pUI->getScreen(0);
						if(pScreen){

							typedef FormatSpecification FS;
							FS requests []  = {
									{FS::rtPixelColorE, FS::pixclRGBA_8888},
									{FS::rtOpenGLRenderable, true},
									{FS::rtDoubleBufferingB, true},
									{FS::rtNativeRenderable, true},
									{FS::rtEnd, 0}
							};
							pWindow = pScreen->createRootWindow(requests);
						}
					}
				}


			}

			if(pWindow && pUI){
				pWindow->changeVisibility(true);
				for(;;){
					pUI->process();
				}
			}
		}


	}


	return 0;
}



