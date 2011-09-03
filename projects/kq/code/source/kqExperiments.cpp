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


		class IResourcer{
		public:
			//The locks can be used as follows
			virtual bool lockResourcer(bool bExclusive) = 0;
			virtual bool unlockResourcer() = 0;
		};

		class IFlowWriter: public IResourcer{
			//When a writer is exclusively locked by a processor it means that only that processor can write to this flow.
			//When a writer is shared locked by a processor multiple processors can atomically write to the flow
		public:
			virtual void write(const void * pBytes, kq::core::ui64 nBytes);
		};

		class IFlowReader: public IResourcer{
		public:
			virtual void read(void * pBytes, kq::core::ui64 nBytes);
		};

		class IFlowTuple: public IResourcer{
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


		class IFlow : public IResourcer{
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
/*
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
*/

static void remoteCall(void * stacklocation, size_t stacksize, void (*fn)(void *), void * data) __attribute__((noinline));
static void remoteCall(void * stacklocation, size_t stacksize, void (*fn)(void *), void * data){

	//Should be optimized at compile time in release
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
	}
}

#include "pthread.h"

#include "stdarg.h"
#include "assert.h"

namespace kq{
	namespace core{
		namespace debug{
			void assume(kq::core::ui32 iVal, ...){
				assert(iVal);
			}

			class Log{
				bool bLogging;
			public:
				Log(){bLogging = false;}
				virtual void enableLogging(){bLogging = true;}
				virtual void disableLogging(){bLogging = false;}
				void log(const char * format, ...){
					va_list args;
					va_start (args, format);
					if(bLogging)
						vprintf (format, args);
					va_end (args);
				}
			};
		}

		namespace system{

			class Resourcer{

			public:
				typedef kq::core::ui32 ResourceCount;

				class ResourceManager:virtual kq::core::debug::Log{
					volatile static kq::core::ui32 nResourcers;
				public:
					ResourceManager(){enableLogging();}
					~ResourceManager(){
						log("%d resourcers leaked\n", nResourcers);
					}
					void operator+=(ResourceCount c){
						nResourcers+=c;
					}
					void operator-=(ResourceCount c){
						nResourcers-=c;
					}
				};

				static ResourceManager manager;
				kq::core::ui32 nResources;
			public:
				Resourcer():nResources(0){}
				~Resourcer(){kq::core::debug::assume(!nResources);}
				void initialize(int iCount = 1){manager+=iCount;nResources+=iCount;}
				void finalize(int iCount = 1){manager-=iCount;nResources-=iCount;}
			};

			class Mutex: virtual public kq::core::debug::Log, private Resourcer{
				pthread_mutex_t mutex;

			public:
				enum Error{
					kErrNone,
					kErrSome,
				};

			protected:
				Error sleep(pthread_cond_t * cond){
					if(0 == pthread_cond_wait(cond, &mutex)){
						return kErrNone;
					}
					return kErrSome;
				}

				friend class Condition;

			public:

				Error initialize(){
					if(0 == pthread_mutex_init(&mutex, 0)){
						Resourcer::initialize();
						return kErrNone;
					}
					return kErrSome;
				}

				void finalize(){
					Resourcer::finalize();
					pthread_mutex_destroy(&mutex);
				}

				Error lock(){
					log("[%p]Lock %p...", pthread_self(), this);
					if(0 == pthread_mutex_lock(&mutex)){
						log("Locked\n");
						Resourcer::initialize();
						return kErrNone;
					}
					printf("Lock Error\n");
					return kErrSome;
				}

				Error unlock(){
					log("[%p]Lock %p...", pthread_self(), this);
					if(0 == pthread_mutex_unlock(&mutex)){
						log("UnLocked\n");
						Resourcer::finalize();
						return kErrNone;
					}
					log("UnLock Error\n");
					return kErrSome;
				}


			};

			class Condition: virtual public kq::core::debug::Log, private Resourcer{
				pthread_cond_t cond;
				bool bWake;

			public:
				enum Error{
					kErrNone,
					kErrSome,
				};
				Error initialize(){
					Error err = kErrSome;
					if(0 == pthread_cond_init(&cond, 0)){
						err = kErrNone;
						Resourcer::initialize();
						goto done;
					}
					done:
					bWake = false;
					return err;
				}

				void finalize(){
					Resourcer::finalize();
					pthread_cond_destroy(&cond);
				}

				Error sleep(Mutex * m){
					Error err = kErrNone;
					bWake = false;
					log("Cond %p and Mutex %p sleeping\n", this, m);
					while(!bWake && err == kErrNone){
						err = (m->sleep(&cond) == Mutex::kErrNone)?kErrNone:kErrSome;
						log("Cond %p and Mutex %p awake\n", this, m);
					}
					log("Cond %p and Mutex %p seperated\n", this, m);
					return err;
				}

				Error wake(){
					log("Cond %p waking\n", this);
					if(0 == pthread_cond_signal(&cond)){
						log("Cond %p woken\n", this);
						bWake = true;
						return kErrNone;
					}
					return kErrSome;
				}

				//TODO: Is wake all compatible with bWake?
				Error wakeAll(){
					if(0 == pthread_cond_broadcast(&cond)){
						bWake = true;
						return kErrNone;
					}
					return kErrSome;
				}
			};

			class ConditionMutex:public Condition,  public Mutex{


			public:
				Condition::Error initialize(){
					Condition::Error err = Condition::kErrSome;
					if(Condition::kErrNone == Condition::initialize()){
						if(Mutex::kErrNone == Mutex::initialize()){
							err = Condition::kErrNone;
							goto done;
						}
						Condition::finalize();
					}
					done:
					return err;
				}

				void finalize(){
					Mutex::finalize();
					Condition::finalize();
				}

				Condition::Error sleep(){
					return Condition::sleep(this);
				}
			};

			class ScopeMutex{
				Mutex * mutex;
			public:
				ScopeMutex(Mutex * mutex){this->mutex = mutex;mutex->lock();}
				~ScopeMutex(){mutex->unlock();}
			};

			class Thread{
			public:
				enum Error{
					kErrNone,
					kErrSome,
				};

				typedef pthread_t Handle;
				static Error create(Handle & t, void * (*fn)(void *), void * data){
					Error ret = kErrSome;
					if(0 == pthread_create(&t, 0, fn, data)){
						ret = kErrNone;
					}
					return ret;
				}

				static Error join(Handle &thread, void ** returnval){
					if(0 == pthread_join(thread, returnval)){
						return kErrNone;
					}
					return kErrSome;
				}

				static Error attach(Handle &thread){
					thread = pthread_self();
					return kErrNone;
				}
			};
		}
		namespace flows{

			enum Error{
				kErrNone,
				kErrPowerOnOff,
				kErrBusy,
				kErrDeadEnd,
				kErrUnexpectedState,
				kErrOutOfMemory,
			};


			class IData{
			protected:
				virtual IData * clone();
				virtual void finalize();
			};

			typedef kq::core::ui32 Port;
			typedef kq::core::ui32 DataCount;
			enum Direction{
				kDirectionIn,
				kDirectionOut,
			};

			struct Message{
				Port port;
				Direction direction;
				IData ** arrData;
				DataCount min;
				DataCount max;
				void set(Port port, Direction d, IData ** arrData, DataCount min, DataCount max);
			};

			class IPorts{
			public:
				virtual Error prefetch(Port iPort, DataCount nData) = 0;
				virtual Error performIO(Message *) = 0;
			};

			typedef Error (* Processor)(const IPorts *, const Message *);

			class ISocket: virtual public kq::core::debug::Log{

			public:
				enum Operation{
					kSocketNone,
					kSocketCreate,
					kSocketInsertProcessor,
					kSocketEjectProcessor,
					kSocketConnect,
					kSocketPrefetch,
					kSocketPerformIO,
					kSocketDestroy,
				};

				struct OperationData{};
				struct OperationCreate:public OperationData{kq::core::memory::MemoryWorker * mem;};
				struct OperationInserProcessor:public OperationData{};

			};

			class DHSSocket:public ISocket{
				kq::core::memory::MemoryWorker & mem;

				kq::core::system::ConditionMutex mx;


				kq::core::system::Condition cCitizen;
				kq::core::system::Thread::Handle hCitizen;
				bool bShuttingDown;
				bool bNewData;
				Operation operation;
				OperationData * operationData;
				Error operationError;

				virtual Error dooperation_citizen(Operation operation, OperationData * data){
					Error err = kErrOutOfMemory;
					log("Operation Done\n");
					return err;
				}


				void citizenwork(){
					{
						kq::core::system::ScopeMutex _0(&this->mx);
						while(!bShuttingDown){
							while(!bNewData && !bShuttingDown)
								cCitizen.sleep(&this->mx);
							if(bShuttingDown){
								break;
							}
							operationError = dooperation_citizen(operation, operationData);
							bNewData = false;
							mx.wake();
						}
					}
					cCitizen.finalize();
				}

				static void * _citizen_work(void * obj){
					((DHSSocket *)obj)->citizenwork();
					return 0;
				}

			public:
				DHSSocket(kq::core::memory::MemoryWorker & memworker):mem(memworker), hCitizen(0){
					mx.initialize();
					mx.enableLogging();
					enableLogging();

				}

				~DHSSocket(){
					bShuttingDown = true;
					{
						kq::core::system::ScopeMutex _0(&this->mx);
						cCitizen.wake();
					}
					if(hCitizen)kq::core::system::Thread::join(hCitizen, 0);
					mx.finalize();
				}

				virtual Error dooperation(Operation operation, OperationData * data){
					Error err;

					if(!hCitizen){
						kq::core::system::ScopeMutex _0(&this->mx);

						if(!hCitizen){
							bShuttingDown = false;
							bNewData = false;
							cCitizen.initialize();
							cCitizen.enableLogging();
							if(kq::core::system::Thread::kErrNone != kq::core::system::Thread::create(hCitizen, &_citizen_work, (void *)this)){
								//Couldnot start citizen
								err = kErrDeadEnd;
								goto done;
							}

						}

					}
					{
						kq::core::system::ScopeMutex _0(&this->mx);
						this->operation = operation;
						this->operationData = data;
						bNewData = true;
						cCitizen.wake();
						while(bNewData){
							mx.sleep();
						}
						err = operationError;


					}
					done:
					log("doOperation Err = %u\n", (unsigned int)err);
					return err;
				}
			};

			class SocketData: public IPorts{
				kq::core::memory::MemoryWorker & mem;

				enum ProcessorState{
					kStateOff,
					kStateOn,
					kStateBusy,
				};
				ProcessorState state;

				Processor processor;
				void * stackstart;
				size_t stacksize;
				kq::core::system::Mutex mxExternalEntry;

				class IPortsProcessor: public IPorts{
					SocketData * socketdata;
					friend class SocketData;
				public:
					virtual Error prefetch(Port iPort, DataCount nData);
					virtual Error performIO(Message *);

				};

				IPortsProcessor processorports;

				Error processOuter(const Message *);
				Error processInner(const Message *);

				kq::core::system::Mutex mxConnections;
				kq::core::data::BPlusTree connections;

			public:
				SocketData(kq::core::memory::MemoryWorker &);
				~SocketData();
				Error ejectProcessor();
				Error insertProcessor(Processor p);
				virtual Error prefetch(Port iPort, DataCount nData);
				virtual Error performIO(Message *);
				Error connect(Port iPort, SocketData * other, Port iOther);

			};
		}
	}
}

volatile kq::core::ui32 kq::core::system::Resourcer::ResourceManager::nResourcers = 0;
kq::core::system::Resourcer::ResourceManager kq::core::system::Resourcer::manager;

using namespace kq;
using namespace kq::core;
using namespace kq::core::memory;
using namespace kq::core::data;
using namespace kq::core::flows;

IData * IData::clone() {return 0;}
void IData::finalize(){}

void Message::set(Port port, Direction d, IData ** arrData, DataCount min, DataCount max)
{
	this->port = port;
	this->arrData = arrData;
	this->min = min;
	this->max = max;
	this->direction = d;
}

SocketData::SocketData(kq::core::memory::MemoryWorker & memworker):
		mem(memworker),
		state(kStateOff),
		processor(0),
		connections(memworker, sizeof(Port))
{
	mxExternalEntry.initialize();
	mxConnections.initialize();
	processorports.socketdata = this;
}

SocketData::~SocketData(){
	ejectProcessor();
}

struct ProcessParcel{
	SocketData * s;
	Error (SocketData::*f)(const Message *);
	Error e;
	const Message * m;
};

Error SocketData::processInner(const Message * m){
	return (*processor)(&processorports, m);
}

void processparceltransfer(ProcessParcel * p){
	//ProcessParcel * p = (ProcessParcel *) v;
	p->e = (p->s->*p->f)(p->m);
}

Error SocketData::processOuter(const Message * m){
	ProcessParcel p;
	p.s = this;
	p.f = &SocketData::processInner;
	p.e = kErrNone;
	p.m = m;

	remoteCall(stackstart, stacksize, (void (*)(void *))processparceltransfer, &p);
	return p.e;
}

Error SocketData::ejectProcessor(){
	Error err;
	mxExternalEntry.lock();
	if(state != kStateOff){
		Message m;
		m.set(0, kDirectionIn, 0, 0, 0);
		if(processOuter(&m) == kErrNone){
			state = kStateOff;
			mem(stackstart, 0);
			err = kErrNone;
		}
		else{
			err = kErrPowerOnOff;
		}
	}
	if(state == kStateOff){
		processor = 0;
		err = kErrNone;
	}
	mxExternalEntry.unlock();

	return err;
}

Error SocketData::insertProcessor(Processor p){
	Error err;
	if(kErrNone == ejectProcessor()){
		processor = p;
	}
	return err;
}

Error SocketData::prefetch(Port iPort, DataCount nData){
	return kErrNone;
}

Error SocketData::performIO(Message * m){
	Port & i = m->port;

	Error err = kErrNone;
	//Port 0 is not open to public
	if(i == 0){
		err = kErrDeadEnd;
	}
	else{
		kq::core::system::ScopeMutex enter(&mxExternalEntry);
		//Check if the processor has booted
		if(state == kStateOff){
			stacksize = 4096;
			stackstart = mem(0, stacksize);
			if(stackstart){
				Message m;
				m.set(0, kDirectionIn, 0, 0, 1);
				err = processOuter(&m);
				if(err == kErrNone){
					state = kStateOn;
				}
				else{
					err = kErrPowerOnOff;
				}
			}
			else{
				err = kErrOutOfMemory;
			}
		}

		if(state == kStateOn){
			state = kStateBusy;
			err = processOuter(m);
			state = kStateOn;
		}
	}

	return err;
}

Error SocketData::IPortsProcessor::prefetch(Port iPort, DataCount nData){
	return kErrNone;
}
Error SocketData::IPortsProcessor::performIO(Message * m){
	return kErrUnexpectedState;
}

Error SocketData::connect(Port iPort, SocketData * other, Port iOther){
	if(iPort == 0 || iOther == 0){
		return kErrDeadEnd;
	}

	//this should be > other, to avoid dead locks
	if(this < other){
		return other->connect(iOther, this, iPort);
	}

	Error err;
	//first make a broad division, make or break?
	if(other){
		//connect/reconnect
		kq::core::data::BPlusTree::Path p(&connections);

	}
	else{
		//disconnect

	}

	return err;

}



/*
class TestProcessor:public Processor{

public:
	TestProcessor(){

	}

	~TestProcessor(){

	}

	struct Return:public IData{
		int ret;
	};

	int entry(){

		Return r;
		IData * pr = &r;
		Flow f;
		f.set(1, 1, 1, &pr);
		if(doFlow(&f) == kErrNone){
			return r.ret;
		}
		return -1;
	}
protected:
	virtual Error handleFlow(const Flow * flow){
		printf("FlowIndex %u Data %p\n", (unsigned int)flow->fid, flow->data);
		return kErrNone;
	}


};

*/

Error consumer(const IPorts * ports, const Message * msg){
	printf("Consumer %p %p[%u %p %u %u]\n", ports, msg, (ui32)msg->port, msg->arrData, (ui32)msg->min, (ui32)msg->max);
	Error err;
	switch(msg->port){
	case 0:
		err = kErrNone;
		break;
	case 1:
	default:
		err = kErrDeadEnd;
	}
	return err;
}

Error producer(const IPorts * ports, const Message * msg){
	printf("Producer %p %p[%u %p %u %u]\n", ports, msg, (ui32)msg->port, msg->arrData, (ui32)msg->min, (ui32)msg->max);
	return kErrNone;

}

int main(int /*argc*/, char ** /*argv*/){
	//LOGINOUT;
	int ret;

	//Create std allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);

	{
		MemoryWorker mem = memStd;
		//PooledMemoryAllocator allocPool(memStd);
		//allocPool.getMemoryWorker(mem);
		{


			/*
			TestProcessor p;
			ret = p.entry();
			*/

			/*
			flows::SocketData p(mem);
			flows::SocketData c(mem);
			p.insertProcessor(producer);
			c.insertProcessor(consumer);
			p.connect(1, &c, 1);
			Message m;
			m.set(1, kDirectionOut, 0, 0, 0);
			c.performIO(&m);
			*/

			flows::DHSSocket dhs(mem);
			flows::Error err = kErrNone;
			{
				DHSSocket::OperationCreate o;
				o.mem = &mem;
				err = dhs.dooperation(DHSSocket::kSocketCreate, &o);
			}

		}
	}



	return ret;
}



