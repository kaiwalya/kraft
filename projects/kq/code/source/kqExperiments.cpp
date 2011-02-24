#include "core.hpp"

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

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

class BPlusTree{

public:

	typedef kq::core::ui16 Large;
	typedef kq::core::ui8 Small;

	void * pRoot;

	const Small nBytesInKey;
	const Large nChildrenPerNode;
	const Small nBitsPerLevel;
	const Small nNibblesPerByte;
	Small * pNibbleMasks;

	//Start(Allocation, Deallocation and UserData)
	kq::core::memory::MemoryWorker mem;
	//End(Allocation, Deallocation and UserData)


	void move(void *** pppCurr, Small & iKeyNibble, kq::core::ui8 * pKey){
		Small nKeyNibbles = nBytesInKey * nNibblesPerByte;
		kq::core::ui8 halfKey;
		while(**pppCurr && (iKeyNibble < nKeyNibbles)){

			Small iByte = iKeyNibble / nNibblesPerByte;
			Small iNibbleID = iKeyNibble % nNibblesPerByte;

			Small & byte = pKey[iByte];
			Small maskedByte = byte & pNibbleMasks[iNibbleID];
			halfKey = (maskedByte >> (nBitsPerLevel * iNibbleID));
			*pppCurr = ((void**)**pppCurr) + halfKey;

			iKeyNibble++;
		}
	}

public:
	BPlusTree(kq::core::memory::MemoryWorker memworker, Small bytesInKey, Small bitsPerLevel = 4)
		:mem(memworker),
		nBytesInKey(bytesInKey),
		nBitsPerLevel(bitsPerLevel),
		nChildrenPerNode(1 << bitsPerLevel),
		nNibblesPerByte(8/bitsPerLevel),
		pRoot(0)
	{

		pNibbleMasks = (Small *)mem(0, sizeof(Small) * nNibblesPerByte);
		{
			Small iBit = 0;
			pNibbleMasks[0] = 0;
			while(iBit < nBitsPerLevel){
				pNibbleMasks[0] = (pNibbleMasks[0] << 1) | 1;				
				iBit++;
			}
			iBit = 1;
			while(iBit < nNibblesPerByte){
				pNibbleMasks[iBit] = pNibbleMasks[iBit - 1] << nBitsPerLevel;
				iBit++;
			}
			
		}

		
	}

	void ** find(void * k){

		kq::core::ui8 * pKey = (kq::core::ui8 *)k;
		Small iKeyNibble = 0;
		Small nKeyNibbles = nBytesInKey * nNibblesPerByte;
		

		void ** ppRet = 0;

		//Try to find store the found node in pCurr and nibbles left in iKeyNibble
		void ** ppCurr = &pRoot;
		{
			/*
			kq::core::ui8 halfKey;
			while(*ppCurr && (iKeyNibble < nKeyNibbles)){

				Small iByte = iKeyNibble / nNibblesPerByte;
				Small iNibbleID = iKeyNibble % nNibblesPerByte;

				Small & byte = pKey[iByte];
				Small maskedByte = byte & pNibbleMasks[iNibbleID];
				halfKey = (maskedByte >> (nBitsPerLevel * iNibbleID));
				ppCurr = ((void**)*ppCurr) + halfKey;

				iKeyNibble++;
			}
			*/
			move(&ppCurr, iKeyNibble, pKey);
		}

		if(iKeyNibble == nKeyNibbles){
			ppRet = ppCurr;
		}
		return ppRet;
		
	}

	void ** findOrCreate(void * k){
		kq::core::ui8 * pKey = (kq::core::ui8 *)k;
		Small iKeyNibble = 0;
		Small nKeyNibbles = nBytesInKey * nNibblesPerByte;
		

		void ** ppRet = 0;

		//Try to find store the found node in pCurr and nibbles left in iKeyNibble
		void ** ppCurr = &pRoot;
		{
			/*
			kq::core::ui8 halfKey;
			while(*ppCurr && (iKeyNibble < nKeyNibbles)){

				Small iByte = iKeyNibble / nNibblesPerByte;
				Small iNibbleID = iKeyNibble % nNibblesPerByte;

				kq::core::ui8 & byte = pKey[iByte];
				kq::core::ui8 maskedByte = byte & pNibbleMasks[iNibbleID];
				halfKey = (maskedByte >> (nBitsPerLevel * iNibbleID));
				ppCurr = ((void**)*ppCurr) + halfKey;

				iKeyNibble++;
			}
			*/
			move(&ppCurr, iKeyNibble, pKey);
		}

		if(iKeyNibble < nKeyNibbles){			
			Large nSizeLevel = sizeof(void *) * nChildrenPerNode;

			void ** pTempLevels = (void **)mem(0, sizeof(void *) * (nKeyNibbles - iKeyNibble));
			if(pTempLevels){
				Small iNibble, nNibbles;
				iNibble = 0;
				nNibbles = nKeyNibbles - iKeyNibble;
				for(iNibble = nNibbles; iNibble > 0 ; iNibble--){
					pTempLevels[iNibble - 1] = mem(0, nSizeLevel);
					if(!pTempLevels[iNibble - 1]){
						break;
					}
					else{
						memset(pTempLevels[iNibble - 1], 0, nSizeLevel);
					}
				}
				if(iNibble != 0){
					while(iNibble < nNibbles) mem(pTempLevels[iNibble++], 0);
				}
				else{					
					iNibble = 0;
					while(iKeyNibble < nKeyNibbles){
						
						
						*ppCurr = pTempLevels[iNibble++];						
						move(&ppCurr, iKeyNibble, pKey);
					}

					ppRet = ppCurr;

				}
				mem(pTempLevels, 0);
			}
		}
		else{
			ppRet = ppCurr;
		}

		return ppRet;
	}

	~BPlusTree(){
		mem(pNibbleMasks, 0);

		
		if(nBytesInKey){
			Small nKeyNibbles = nBytesInKey * nNibblesPerByte;
			
			void ** stackNode = (void**)mem(0, nKeyNibbles * sizeof(void **));
			Large * stackIndex = (Large *)mem(0, nKeyNibbles * sizeof(Large));
			Small top = nKeyNibbles - 1;

			stackNode[top] = pRoot;
			stackIndex[top] = 0;

			void ** pCurr;		
			while((pCurr = (void**)stackNode[top]) != 0){			
				Large iCurrChild = stackIndex[top];

				if(iCurrChild < nChildrenPerNode && top){					
					void * pNext = (void*)(pCurr[iCurrChild]);
					stackIndex[top]++;
					pCurr[iCurrChild] = 0;
					if(pNext){
						--top;
						stackNode[top] = pNext;
						stackIndex[top] = 0;
					}
				}
				else{
					
					mem(pCurr, 0);
					if(top < nKeyNibbles - 1){
						top++;
					}
					else{
						mem(stackNode,0);
						mem(stackIndex, 0);
						break;
					}
				}
			}
		}
	}
};

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


int main(int /*argc*/, char ** /*argv*/){
	LOGINOUT;
	
	//Create std allocator
	StandardLibraryMemoryAllocator allocStd;
	MemoryWorker memStd;
	allocStd.getMemoryWorker(memStd);


	{
		PooledMemoryAllocator allocPool(memStd);
		MemoryWorker mem;
		allocPool.getMemoryWorker(mem);

		
		/*
		flows::Machine m(mem);

		{
			flows::Variable a,b;
			kq::core::memory::RefCounter * p = kq_core_memory_workerRefCountedClassNew(mem, Input);
			//p->increment();
			Pointer<Input> pInput = p;
			Pointer<Output> pOutput = kq_core_memory_workerRefCountedClassNew(mem, Output);
			a = m.variableFromProcessor(pInput.castStatic<IProcessor>());
			b = m.variableFromProcessor(pOutput.castStatic<IProcessor>());

			a >> b;
		}

		Test * arrInt = kq_core_memory_workerArrayNew(mem, Test, 3);
		kq_core_memory_workerArrayDelete(mem, Test, arrInt);

		
		{
			Pointer<Test> pArr (kq_core_memory_workerRefCountedArrayNew(mem, Test, 3));
			pArr += 2;
			pArr[-2].a = 0;
			pArr[-1].a = 1;
			(pArr + 0)->a = 2;
		}
		*/
		

		{
			typedef char test_t;
			BPlusTree bpt(mem, sizeof(test_t), 4);

			test_t iKey = -1;
			kq::core::ui16 iIter = 0;

			srand(0);
			while(++iIter){

				*(int *)(bpt.findOrCreate(&iKey)) = iKey;
				
				int iFindChoice = rand();

				test_t * pInt;
				if(iFindChoice & 0x1){
					pInt = (test_t *)(bpt.find(&iKey));
				}
				else{
					pInt = (test_t *)(bpt.findOrCreate(&iKey));
				}
				test_t iTemp = (rand()%256);
				(*pInt) += iTemp;
				iKey += iTemp;

				if(iKey != *pInt){
					break;
				}
			}
		}

	}
	return 0;
}
