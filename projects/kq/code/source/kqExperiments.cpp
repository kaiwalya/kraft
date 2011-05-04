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
#include "setjmp.h"

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

static void * remoteCall(void * stacklocation, size_t stacksize, void * (*fn)(void *), void * data) __attribute__((noinline));
static void * remoteCall(void * stacklocation, size_t stacksize, void * (*fn)(void *), void * data){

	//If is optimized at compile time in release
	if(sizeof(void *) == sizeof(kq::core::ui64)){

		//Prepare stack
		{
			volatile register kq::core::ui64 sp asm("rsp");
			sp = (kq::core::ui64)stacklocation + stacksize;
			//Dont know why the compiler optimizes the following line out?
			//sp = sp & 0xFFFFFFFFFFFFFFF0;
		}

		fn(data);

		//Undo stack effects
		{
			//Without this last line leave instruction calls are wrong
			//Also makes sure that in release builds, fn(data) is not a tail call
			//If this is not present leave instruction will execute before the fn call and restore the stack
			volatile register kq::core::ui64 bp asm("rbp");
			volatile register kq::core::ui64 sp asm("rsp");
			sp = bp;
		}
	}

}

namespace kq{
	namespace core{
		namespace data{
			class IDMap{
				memory::MemoryWorker & mem;
				BPlusTree tree;
				typedef ui32 IDVal;
				IDVal m_iNext;
				IDVal m_nAlive;
			public:
				IDMap(memory::MemoryWorker & memworker)
				:mem(memworker), tree(memworker, sizeof(IDVal)), m_iNext(1), m_nAlive(0){}

				~IDMap(){}

				IDVal getAliveIDCount(){return m_nAlive;}

				template<typename t> IDVal create(t * p){
					IDVal ret = 0;
					if(tree.map(&(m_iNext), p)){
						ret = m_iNext;
						m_nAlive++;
						m_iNext++;
					}
					return ret;
				}

				template<typename t> t * get(const IDVal id){
					t * ret = 0;
					if(id && id < m_iNext){
						ret = tree.lookup(&id);
					}
					return ret;
				}

				template<typename t> t * destroy(const IDVal id){
					t * ret = 0;
					if(id && id < m_iNext){
						if(tree.map(&id, 0, (void **)&ret)){
							m_nAlive--;
						}
					}
					return ret;
				}

			public:
				kq::core::data::BPlusTree::Path getIterator(){
					return kq::core::data::BPlusTree::Path(&tree);
				}

			};

		}
		namespace flows{

			typedef kq::core::ui8 Data;
			//typedef kq::core::ui8 ID;

			class IReader{
			public:
				virtual kq::core::memory::Pointer<Data> read() = 0;
				virtual bool next() = 0;
			};

			class IWriter{
			public:
				virtual bool write(kq::core::memory::Pointer<kq::core::ui8> pData) = 0;
			};

			class Stream:public IWriter{
			protected:

				class Message{
				public:
					kq::core::memory::Pointer<Message> pNext;
					kq::core::memory::Pointer<Data> pData;

				};

			public:
				class Reader:public IReader{
					friend class Stream;
					kq::core::memory::Pointer<Message> m_pCurrent;
				public:
					kq::core::memory::Pointer<Data> read(){
						return m_pCurrent->pData;
					}

					bool next(){
						//printf("Reader %p next entered\n", this);
						bool bRet = false;
						if(m_pCurrent->pNext){
							m_pCurrent = m_pCurrent->pNext;
							bRet = true;
						}
						//printf("Reader %p next leaving\n", this);
						return bRet;
					}
				};
			protected:
				kq::core::memory::MemoryWorker &mem;
				kq::core::memory::Pointer<Message> m_pLast;
			public:
				Stream(kq::core::memory::MemoryWorker &memworker):mem(memworker){
					m_pLast = kq_core_memory_workerRefCountedClassNew(mem, Message);
				}

				virtual ~Stream(){}

				virtual bool write(kq::core::memory::Pointer<kq::core::ui8> pData){
					if(pData){
						kq::core::memory::Pointer<Message> pMessage = kq_core_memory_workerRefCountedClassNew(this->mem, Message);
						if(pMessage){
							pMessage->pData = pData;
							m_pLast->pNext = pMessage;
							m_pLast = pMessage;
							return true;
						}
					}
					return false;
				}

			protected:
				virtual kq::core::memory::Pointer<Reader> createReader(){
					kq::core::memory::Pointer<Reader> pRet;
					pRet = (kq_core_memory_workerRefCountedClassNew(mem, Reader));
					if(pRet){
						pRet->m_pCurrent = m_pLast;
					}
					return pRet;
				}

			};

			typedef void (*Processor)(void);
			typedef ui32 PinID;
			typedef ui32 SocketID;

			struct Pin{
				const SocketID sockid;
				const PinID pinid;
				Pin(SocketID s, PinID p):sockid(s), pinid(p){}
				bool operator >>(Pin dest){
					return false;
				}
			};

			struct Socket{
			public:
				const SocketID id;
				Socket(SocketID sockid = 0):id(sockid){};
				Pin operator [] (PinID pid){return Pin(id, pid);}

			};

			class Board{

				class BoardState{

					struct ProcessorInfo{

						Processor proc;


						SocketID sockid;
						BoardState * boardstate;

						enum ProcessorState{

						};
						void * stacklocation;
						size_t stacksize;

						jmp_buf * context[2];
					};

					kq::core::memory::MemoryWorker & mem;
					Board * board;
					friend class Board;
					kq::core::data::IDMap map;

					void run(ProcessorInfo * info){
						int iRet = setjmp(*info->context[1]);
						if(iRet == 0){
							longjmp(*info->context[0], 1);
						}
					}

					void harness(ProcessorInfo * info){


						int iRet = setjmp(*info->context[0]);
						if(iRet != 0){
							//Dont have any hopes of returning from here
							{
								volatile register void ** bp asm("rbp");
								*bp = 0;
							}

							info->proc();

							mem(info->context[0], 0);
							info->context[0] = info->context[1];

							//Stack on which this function is executing will be gone
							mem(info->stacklocation, 0);
							info->stacklocation = 0;
							longjmp(*info->context[1], 1);

						}

					}
					static void * _harness(void * data){
						ProcessorInfo * info = (ProcessorInfo *)data;
						info->boardstate->harness(info);
						return 0;
					}

					void finalizeProcessorInfo(ProcessorInfo * pInfo){
						printf("BoardState(%p)::finalizeProcessorInfo(%u)\n", this, pInfo->sockid);
						run(pInfo);
						mem(pInfo, 0);
					}
				public:
					BoardState(kq::core::memory::MemoryWorker & memworker):mem(memworker), map(memworker){
						printf("BoardState(%p)::BoardState()\n", this);
					}

					~BoardState(){
						kq::core::data::BPlusTree::Path p = map.getIterator();
						ProcessorInfo * pInfo;
						if(p.init_first((void **)&(pInfo))){
							do{
								finalizeProcessorInfo(pInfo);
							}while(p.next((void **)&(pInfo)));
						}
						printf("BoardState(%p)::~BoardState()\n", this);
					}

					Socket attach(Processor proc){
						SocketID ret = 0;
						const size_t stacksize = 1024;

						size_t sz[] = {sizeof(ProcessorInfo), stacksize, sizeof(jmp_buf), sizeof(jmp_buf)};
						const int n = sizeof(sz)/sizeof(sz[0]);
						void * ptr[n];
						int i = 0;
						while(i < n){
							ptr[i] = mem(0, sz[i]);
							if(!ptr[i]){
								break;
							}
							i++;
						}
						if(i == n){

							ProcessorInfo * info = (ProcessorInfo *)ptr[0];

							info->proc = proc;
							info->sockid = (SocketID)(map.create(info));
							ret = info->sockid;

							info->stacklocation = ptr[1];
							info->stacksize = stacksize;

							info->context[0] = (jmp_buf *)ptr[2];
							info->context[1] = (jmp_buf *)ptr[3];

							info->boardstate = this;

							remoteCall(info->stacklocation, info->stacksize, _harness, info);

						}
						else{

							while(i){
								i--;
								mem(ptr[i], 0);
							}
						}

						return Socket(ret);
					}
				};

				BoardState * state;

				static Board * gRoot;
				static Board * findBoard(){
					Board * ret = 0;
					ret = gRoot;
					return ret;
				}

				static BoardState * findState(){
					Board * board = findBoard();
					if(board){
						return board->state;
					}
					return 0;
				}

			public:
				Board():state(0){
					printf("Board(%p)::Board()\n", this);
				}

				~Board(){
					printf("Board(%p)::~Board()\n", this);
				}
				static bool initialize(kq::core::memory::MemoryWorker & memworker){
					Board * board = findBoard();
					if(board){
						if(!board->state){
							board->state = (BoardState *)memworker(0, sizeof(BoardState));
							if(board->state){
								new (board->state) BoardState(memworker);
								board->state->board = board;
								return true;
							}
						}
					}
					return false;
				}

				static bool finalize(){
					Board * board = findBoard();
					if(board){
						if(board->state){
							kq::core::memory::MemoryWorker & mem = board->state->mem;
							board->state->~BoardState();
							mem(board->state, 0);
							return true;
						}
					}
					return false;
				}

				static Socket attach(Processor proc){
					BoardState * s = findState();
					if(s) return s->attach(proc);
					return Socket();
				}
			};
		}
	}
}

static kq::core::flows::Board rootboard;
kq::core::flows::Board * kq::core::flows::Board::gRoot = &rootboard;


using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;
using namespace kq::core::data;
using namespace kq::core::flows;

void producer(){
	printf("In producer\n");
	printf("Out producer\n");
}

void consumer(){
	printf("In consumer\n");
	printf("Out consumer\n");
}




int main(int /*argc*/, char ** /*argv*/){
	//LOGINOUT;

	//Create std allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	/*
	void * pStackTop = mem(0, 1024);
	void * pStackBase = (ui8*)pStackBottom + 102;
	void * pRet = rc(pStackTop, test, (void *)0x00FF00FF);
	printf("test returned %p\n", pRet);
	*/


	{
		MemoryWorker mem = memStd;
		//PooledMemoryAllocator allocPool(memStd);
		//allocPool.getMemoryWorker(mem);
		{

			if(Board::initialize(mem)){
				Socket p = Board::attach(&producer);
				Socket c = Board::attach(&consumer);
				if(p.id && c.id){
					if(p[0] >> c[0]){
						printf("Link Successful\n");
					}
				}
				Board::finalize();
			}

		}
	}



	return 0;
}



