#include "core.hpp"

#include "stdlib.h"
#include "time.h"
#include "crtdbg.h"
#include "stdio.h"
#include "windows.h"


using namespace kq::core;
using namespace kq::core::memory;


void test(MemoryWorker alloc, ui32 iSeq){
	ui64 nAllocs = 0x1000;
	ui64 nReallocCycles = 0x100;


	ui64 i;
	ui64 r;
	void * pNext = 0;
	//unsigned int iSeq = (unsigned)(time(0));
	//printf("Test Sequence ID: %d, %x\n", iSeq, iSeq);
	srand(iSeq);			

	i = 0;
	while(i < nAllocs){

		r = (((unsigned)rand()) % (PooledMemoryAllocator::nMaxSize));
		void * pTemp = alloc(0, r + sizeof(void *));
		{
			ui8 * p = ((ui8 *)pTemp) + sizeof(void *);
			ui64 n = r;
			ui64 i = 0;
			while(i < n){
				*p = (i%256);
				i++;
				p++;
			}
		}

		(*((void **)pTemp)) = pNext;
		pNext = pTemp;
		//(!(i % (nAllocs/16)))?printf("+", pNext):0;
		i++;
	};
	//printf("\n");
	
	i = 0;
	while(i < nReallocCycles){
		void ** pCur = &pNext;
		ui32 cnt = 0;
		while(*pCur){

			//r = (((unsigned)rand()) % (PooledMemoryAllocator::nMaxSize));					
			r = (((unsigned)rand()) % (PooledMemoryAllocator::nMaxSize));
			//r = (((unsigned)rand()) % (256) + 900);	

			//printf("%p, ", *pCur);
			*pCur = alloc(*pCur, r + sizeof(void *));
			//printf("%p\n", *pCur);
			ui8 * p = ((ui8 *)*pCur) + sizeof(void *);
			ui64 n = r;
			ui64 i = 0;
			while(i < n){
				*p = (i%256);
				i++;
				p++;
			}

			pCur = *(void ***)pCur;					

			//(!(cnt % (nAllocs/16)))?printf("=", pNext):0;
			cnt++;
		}
		//printf("\n");
		//printf("=");
		i++;				
	}
	//printf("\n");

	
	i = 0;
	while(pNext){
		void * pTemp = *(void **)pNext;				
		alloc(pNext, 0);
		pNext = pTemp;
		//(!(i % nAllocs/16))?printf("-", pNext):0;
		i++;
	}
	//printf("\n");

}


void testAlloc(ui32 iSeq){
	StandardLibraryMemoryAllocator allocator;	

	MemoryWorker alloc;
	allocator.getMemoryWorker(alloc);

	test(alloc, iSeq);

}



void testPoolAlloc(ui32 iSeq){
	StandardLibraryMemoryAllocator allocator;	

	MemoryWorker alloc;
	allocator.getMemoryWorker(alloc);

	MemoryWorker poolalloc = alloc;
	PooledMemoryAllocator poolallocator(alloc);
	poolallocator.getMemoryWorker(poolalloc);						

	test(poolalloc, iSeq);

}

float timeFunc(void (*pfn)(ui32 iSeq), ui32 iSeq){
	float d = (float)GetTickCount();
	(*pfn)(iSeq);
	d = (float)GetTickCount() - d;
	return d;
};


template<typename t>
struct DynamicArray{
	ui32 m_nElementsAllocated;
	ui32 m_nElements;
	t * m_pElements;


	MemoryWorker & memory;

	DynamicArray(MemoryWorker & allocator):memory(allocator){
		m_nElements = 0;
		m_pElements = 0;
		m_nElementsAllocated = 0;
	};

	virtual ~DynamicArray(){
		setArraySize(0);
	}

	t * getElementLocation(ui32 index){
		if(index <= m_nElements){
			return (m_pElements + index);
		}
		return 0;
	};

	const t * getElementLocation(ui32 index)const{
		if(index <= m_nElements){
			return (m_pElements + index);
		}
		return 0;
	};

	
	t & operator [](ui32 index){
		return *(getElementLocation(index));
	};

	
	const t & operator[](ui32 index) const{
		return *(getElementLocation(index));
	}

	void setArraySize(ui32 nElements){		
		if(m_nElementsAllocated < nElements || m_nElementsAllocated >= (nElements * 2)){
			ui32 nAllocElements = 0;
			if(nElements){
				//Compute the ceil(log2(nElements));
				ui32 n = nElements - 1;
				nAllocElements = 1;

				while(n){
					nAllocElements = nAllocElements << 1;
					n = n >> 1;
				}
			}

			if(m_nElementsAllocated != nAllocElements){
				t * pElements = (t *)memory(m_pElements, nAllocElements * sizeof(t));
				m_pElements = pElements;
				m_nElementsAllocated = nAllocElements;
			}
		}

		m_nElements = nElements;
		
	}

	ui32 getArraySize() const{
		return m_nElements;
	}

	void zeroElements(ui32 iFrom, ui32 iTo){
		if(iTo < iFrom){
			zeroElements(iTo, iFrom):
		}
		if(iTo >= m_nElements){
			zeroElements(iFrom, m_nElements - 1);
		}
		void * pFrom = m_pElements + iFrom;
		ui32 nBytes = (iTo - iFrom + 1)*sizeof(t);
		memset(pFrom, 0, nBytes);
	}
};

//Null terminated String
//Memory block/stream
//Keyboard stream blocking
//Keyboard stream non blocking

template <typename Unit>
struct ICursor{
	virtual i64 readIntoMemory(Unit * pUnit, i64 nUnits){
		return 0;
	};
	virtual i64 writeFromMemory(const Unit * pUnit, i64 nUnits){
		return 0;
	}
	virtual i64 rewind(i64 nUnits){
		return false;
	}
	virtual i64 forward(i64 nUnits){
		return false;
	};
};

template<typename DataType>
struct Map{
	struct MapNode{
		MapNode * arrChild[256];
		ui8 nChildren;

		MapNode * pParent;
		ui8 iParentIndex;		

		DataType pData;

		void initialize(){
			

			pParent = 0;
			iParentIndex = 0;

			memset(arrChild, 0, sizeof(arrChild));
			memset(&pData, 0, sizeof(pData));
			nChildren = 0;
		}

	};	

	typedef ICursor<ui8> Key;

	MapNode m_root;
	MemoryWorker & wMem;

	Map(MemoryWorker & alloc):wMem(alloc){
		m_root.initialize();
	}

	~Map(){
		//DFS deletion
		MapNode * pNode = &m_root;
		while(pNode){
			//If node has children we want to go
			//to them first
			if(pNode->nChildren){
				//Find first child
				MapNode ** pFirstChild = pNode->arrChild;
				while(!(*pFirstChild)){
					pFirstChild++;
				}
				pNode = *pFirstChild;
			}
			else{
				//No children, this node can be safely deleted
				MapNode * pParent = pNode->pParent;
				if(pParent){
					//Delete only if parent is present
					//Else we might delete the root node
					pParent->arrChild[pNode->iParentIndex] = 0;
					pParent->nChildren--;

					wMem(pNode, 0);					
				}
				pNode = pParent;
			}
		}
	};

	void traverse(MapNode * & pNode, Key & k){
		ui8 unit;
		MapNode * pNext;		
		while((k.readIntoMemory(&unit, 1) == 1) && ((pNext = pNode->arrChild[unit]) != 0)){
			pNode = pNext;
			k.forward(1);
		};
	}

	DataType set(MapNode * & pNode, Key & k, DataType pData){
		DataType pRet = 0;
		traverse(pNode, k);
		ui8 unit;
		while(k.readIntoMemory(&unit, 1) == 1){
			MapNode * pNewNode = (MapNode *)wMem(0, sizeof(MapNode));
			pNewNode->initialize();

			pNode->arrChild[unit] = pNewNode;
			pNode->nChildren++;

			pNewNode->pParent = pNode;
			pNewNode->iParentIndex = unit;

			pNode = pNewNode;
			k.forward(1);
		};

		pRet = pNode->pData;
		pNode->pData = pData;
		return pRet;
	}

	DataType reset(MapNode * & pNode, Key & k){
		traverse(pNode, k);

		DataType pRet = 0;
		//Check if k has ended, we dont proceed if the key
		//is not found
		if(k.forward(1) == 0){
			//if ended, the node is the correct node			

			//remove data for the current node
			pRet = pNode->pData;
			pNode->pData = 0;
			
			//We want to leave the nodes if they have children
			//or they have data
			while(pNode && !pNode->nChildren && !pNode->pData){
				MapNode * pParent = pNode->pParent;

				if(pParent){
					pParent->arrChild[pNode->iParentIndex] = 0;
					pParent->nChildren--;
					
					//delete only if you have a parent
					//else we may delete the root node
					wMem(pNode, 0);
				}else{
					int j;
					j = 0;
				}
				
				pNode = pParent;
			}
		}
		return pRet;
	}

	DataType map(Key & k, DataType pData){
		DataType pRet;
		MapNode * pTemp = &m_root;
		if(!pData){
			pRet = reset(pTemp, k);
		}else{
			pRet = set(pTemp, k, pData);
		}
		return pRet;
	};

	DataType lookup(Key & k){
		DataType pRet = 0;
		MapNode * pTemp = &m_root;
		traverse(pTemp, k);
		if(k.forward(1) == 0){
			pRet =  pTemp->pData;
		}

		return pRet;
	}

	void operator = (Map<DataType> &){
	}
	
};

template <typename Unit>
class NullTerminatedStringReadCursor:public ICursor<Unit>{
public:
	Unit * m_pStart;
	Unit * m_pCurrent;
	NullTerminatedStringReadCursor(Unit * pString){
		m_pStart = (Unit *)pString;
		m_pCurrent = m_pStart;
	}

	i64 readIntoMemory(Unit * pUnit, i64 nUnits){
		i64 iRet = 0;
		while(*(m_pCurrent + iRet) && nUnits){
			*pUnit = *(m_pCurrent + iRet);

			nUnits--;			
			iRet++;
		}
		return iRet;
	};

	i64 writeFromMemory(const Unit * pUnit, i64 nUnits){
		return 0;
	}

	i64 rewind(i64 nUnits){
		i64 iRet = 0;
		if(nUnits < 0){
			iRet = forward(-nUnits);
		}
		else{
			Unit * pCurrent = m_pCurrent;
			pCurrent -= nUnits;
			if(pCurrent < m_pStart){
				pCurrent = m_pStart;
			}
			iRet = m_pCurrent - pCurrent;
			m_pCurrent = pCurrent;
		}
		return iRet;
	}

	i64 forward(i64 nUnits){
		i64 iRet = 0;
		if(nUnits < 0){
			iRet = rewind(-nUnits);
		}
		else{
			i64 iRet = 0;
			do{
				m_pCurrent++;
				iRet++;
				nUnits--;
			}while(*m_pCurrent && nUnits);
		}
		return iRet;
	};
};


class IOutputStream{
	ui64 write(void * pBytes, ui64 nBytes);
};

class IGameState{

};


int main_other(int , char **){

	

	{
	};
	

	{
		StandardLibraryMemoryAllocator alloc;	
		MemoryWorker wAlloc;
		alloc.getMemoryWorker(wAlloc);

		PooledMemoryAllocator poolalloc(wAlloc);
		MemoryWorker wPoolAlloc;
		poolalloc.getMemoryWorker(wPoolAlloc);
	
		/*	
		
		ui8 s1[] = "";
		ui8 s2[] = "a";
		ui8 s3[] = "aa";
		ui8 s4[] = "ab";
		ui8 s5[] = "aaa";
		ui8 s6[] = "aab";
		ui8 s7[] = "aba";
		ui8 s8[] = "abb";
		Map<ui8 *> m(wPoolAlloc);

		
		m.map(NullTerminatedStringReadCursor<ui8>(s1), s1);
		m.map(NullTerminatedStringReadCursor<ui8>(s2), s2);
		m.map(NullTerminatedStringReadCursor<ui8>(s3), s3);
		m.map(NullTerminatedStringReadCursor<ui8>(s4), s4);
		m.map(NullTerminatedStringReadCursor<ui8>(s5), s5);
		m.map(NullTerminatedStringReadCursor<ui8>(s6), s6);
		m.map(NullTerminatedStringReadCursor<ui8>(s7), s7);
		m.map(NullTerminatedStringReadCursor<ui8>(s8), s8);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s1)), s1);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s2)), s2);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s3)), s3);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s4)), s4);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s5)), s5);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s6)), s6);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s7)), s7);
		printf("%s = %s\n", m.lookup(NullTerminatedStringReadCursor<ui8>(s8)), s8);
		//m.map(NullTerminatedStringReadCursor<ui8>(s1), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s2), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s3), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s4), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s5), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s6), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s7), 0);
		//m.map(NullTerminatedStringReadCursor<ui8>(s8), 0);
		
		*/

		//ui32 iSeq = (ui32)time(0);
		//printf("Speedup = %f\n", timeFunc(testAlloc, iSeq)/timeFunc(testPoolAlloc, iSeq));

	}
	_CrtDumpMemoryLeaks();	
	
	return 0;
};


