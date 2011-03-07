#include "core_data_BPlusTree.hpp"


#include "stdio.h"
#include "stdlib.h"
#include "string.h"

using namespace kq;
using namespace kq::core;
using namespace kq::core::data;

BPlusTree::BPlusTree(kq::core::memory::MemoryWorker memworker, USmall bytesInKey, USmall bitsPerLevel)
	:mem(memworker),
	m_nBytesInKey(bytesInKey),
	m_nBitsPerLevel(bitsPerLevel),
	m_nChildrenPerNode(1 << bitsPerLevel),
	m_nNibblesPerByte(8/bitsPerLevel),
	m_nNibblesInKey(m_nBytesInKey * m_nNibblesPerByte),
	m_nLevelsInStack(m_nNibblesInKey + 1),
	m_nSizeOfStack(m_nLevelsInStack * sizeof(void *)),
	m_pRoot(0)
{

	//LOGINOUT;
	//m_pNibbleMasks = (USmall *)mem(0, sizeof(USmall) * m_nNibblesPerByte);
	{
		USmall iBit = 0;
		m_pNibbleMasks[0] = 0;
		while(iBit < m_nBitsPerLevel){
			m_pNibbleMasks[0] = (m_pNibbleMasks[0] << 1) | 1;
			iBit++;
		}
		iBit = 1;
		while(iBit < m_nNibblesPerByte){
			m_pNibbleMasks[iBit] = m_pNibbleMasks[iBit - 1] << m_nBitsPerLevel;
			iBit++;
		}
		
	}

	
}


BPlusTree::~BPlusTree(){
	//LOGINOUT;
	//mem(m_pNibbleMasks, 0);

	/*
	
	kq::core::ui64 iLeftForDead = 0;

	if(m_nBytesInKey && m_pRoot){
		USmall nKeyNibbles = m_nNibblesInKey;

		void ** stackNode = (void**)mem(0, nKeyNibbles * sizeof(void **));
		ULarge * stackIndex = (ULarge *)mem(0, nKeyNibbles * sizeof(ULarge));
		USmall top = nKeyNibbles - 1;

		stackNode[top] = m_pRoot;
		stackIndex[top] = 0;

		void ** pCurr;
		while((pCurr = (void**)stackNode[top]) != 0){
			ULarge iCurrChild = stackIndex[top];

			if(iCurrChild < m_nChildrenPerNode && top){
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
				iLeftForDead++;
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

	*/
	
	

	Path p1(this);
	Path p2(this);
	if(p1.init_first()){
		p2 = p1;
		while(p2.next()){
			p1.write(0);
			p1 = p2;
		}
		p1.write(0);
	}

}


bool BPlusTree::map(void * key, void * newV, void ** oldV){
	bool bRet = false;
	if(newV){
		void ** pLocation = findOrCreate(key);
		if(pLocation){
			if(oldV){
				*oldV = *pLocation;
			}
			*pLocation = newV;
			bRet = true;
		}else{
			int k;
			k = 0;
		}
	}
	else{
		if(oldV){
			void ** pLocation = find(key);
			if(pLocation){
				*oldV = *pLocation;
			}
		}
		bRet = destroy(key);
	}

	return bRet;
}

void * BPlusTree::lookup(void * key){
	void ** loc = find(key);
	if(loc){
		return *loc;
	}
	return 0;
}

void ** BPlusTree::find(void * k){

	//LOGINOUT;
	kq::core::ui8 * pKey = (kq::core::ui8 *)k;
	USmall iKeyNibble = 0;
	USmall nKeyNibbles = m_nNibblesInKey;
	

	void ** ppRet = 0;

	//Try to find store the found node in pCurr and nibbles left in iKeyNibble
	void ** ppCurr = &m_pRoot;
	move(&ppCurr, iKeyNibble, pKey);
	if(iKeyNibble == nKeyNibbles){
		ppRet = ppCurr;
	}
	return ppRet;
	
}

void ** BPlusTree::findOrCreate(void * k){
	//LOGINOUT;
	kq::core::ui8 * pKey = (kq::core::ui8 *)k;
	USmall iKeyNibble = 0;
	USmall nKeyNibbles = m_nNibblesInKey;
	

	void ** ppRet = 0;

	//Try to find store the found node in pCurr and nibbles left in iKeyNibble
	void ** ppCurr = &m_pRoot;
	move(&ppCurr, iKeyNibble, pKey);

	if(iKeyNibble < nKeyNibbles){			
		ULarge nSizeLevel = sizeof(void *) * m_nChildrenPerNode;

		void ** pTempLevels = (void **)mem(0, sizeof(void *) * (nKeyNibbles - iKeyNibble));
		if(pTempLevels){
			USmall iNibble, nNibbles;
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

bool BPlusTree::destroyStack(Stack s, void ** oldValue){
	if(oldValue){
		*oldValue = *s[m_nLevelsInStack - 1];
	}
	*s[m_nLevelsInStack - 1] = 0;

	bool bRet = true;
	ULarge iChild;
	ULarge nDeadChildren;

	USmall iNibble = m_nLevelsInStack - 1;
	while(iNibble > 0){
		iChild = 0;
		nDeadChildren = 0;
		void ** ppFirstChild = (void **)*s[iNibble-1];

		while(iChild < m_nChildrenPerNode){
			if(!ppFirstChild[iChild]){
				nDeadChildren++;
				iChild++;
				continue;
			}
			break;
		}
		if(nDeadChildren != m_nChildrenPerNode){
			break;
		}
		//LOGDEPTH;
		//printf("*(%p) [%p -> 0]\n", pppPath[iNibble-1], *pppPath[iNibble-1]);
		mem(ppFirstChild, 0);
		*s[iNibble-1] = 0;

		iNibble--;
	}
	return bRet;
}

bool BPlusTree::destroy(void * pKey, void ** oldValue){
	//LOGINOUT;
	bool bRet = false;
	Stack pppPath = stackCreate();
	if(pppPath){		
		if(getStackFromKey(pppPath, pKey)){
			if(destroyStack(pppPath, oldValue)){
				bRet = true;
			}
		}
		mem(pppPath, 0);
	}

	return bRet;
}

void BPlusTree::move(void *** pppCurr, USmall & iKeyNibble, kq::core::ui8 * pKey){

	USmall nKeyNibbles = m_nNibblesInKey;
	USmall iInnerKeyNibble = m_nNibblesInKey - iKeyNibble - 1;
	kq::core::ui8 halfKey;


	while(**pppCurr && (iKeyNibble < nKeyNibbles)){

		USmall iByte = iInnerKeyNibble / m_nNibblesPerByte;
		USmall iNibbleID = iInnerKeyNibble % m_nNibblesPerByte;

		USmall & byte = pKey[iByte];
		USmall maskedByte = byte & m_pNibbleMasks[iNibbleID];
		halfKey = (maskedByte >> (m_nBitsPerLevel * (iNibbleID)));
		*pppCurr = ((void**)**pppCurr) + halfKey;

		iKeyNibble++;
		iInnerKeyNibble--;
	}


}


BPlusTree::Stack BPlusTree::stackCreate(){
	const ULarge sz = m_nSizeOfStack;
	BPlusTree::Stack s = (BPlusTree::Stack)mem(0, sz);
	if(s){
		memset(s + 1, 0, sz - sizeof(s[0]));
		s[0] = &m_pRoot;
	}
	return s;
}
void BPlusTree::stackDestroy(BPlusTree::Stack s){
	mem(s, 0);
}

void BPlusTree::stackCopy(kq::core::data::BPlusTree::Stack sOut, kq::core::data::BPlusTree::Stack sIn){
	memcpy(sOut, sIn, m_nSizeOfStack);
}



void BPlusTree::getKeyFromStack(void * k, BPlusTree::Stack s){

	USmall iNibble = 0;

	kq::core::ui8 * key = (kq::core::ui8 *)k;
	memset(key, 0, m_nBytesInKey);
	while(iNibble < m_nNibblesInKey && s[m_nLevelsInStack - iNibble - 1]){

		USmall iByte = (USmall)((iNibble) / m_nNibblesPerByte);
		USmall iNID = iNibble % m_nNibblesPerByte;
		USmall &byte = key[iByte];


		USmall halfKey = (USmall)(s[m_nLevelsInStack - iNibble - 1] - (void **)*s[m_nLevelsInStack - iNibble - 1 - 1]);
		USmall maskedByte = halfKey << (m_nBitsPerLevel * iNID);
		byte = byte | maskedByte;
		iNibble++;
	}

};

bool BPlusTree::getStackFromKey(BPlusTree::Stack s, void * k){
	const ULarge nNibbles = m_nNibblesInKey;
	USmall iNibble = 0;
	bool bRet = false;
	BPlusTree::Stack stack = stackCreate();
	if(stack){
		path(stack, iNibble, (kq::core::ui8 *)k);
		if(iNibble == nNibbles){
			bRet = true;
			memcpy(s, stack, m_nSizeOfStack);
		}
		stackDestroy(stack);
	}
	return bRet;
}

void * BPlusTree::readValueAtStack(BPlusTree::Stack s){
	const ULarge nNibbles = m_nNibblesInKey;
	return *s[nNibbles];
}

void BPlusTree::writeValueAtStack(BPlusTree::Stack s, void * v){
	if(!v){
		destroyStack(s, 0);
	}else{
		const ULarge nNibbles = m_nNibblesInKey;
		*s[nNibbles] = v;
	}
	
}


bool BPlusTree::nextleaf(BPlusTree::Stack s, SSmall iDirection){
	bool bRet = false;
	ULarge nNibbles = m_nNibblesInKey;
	ULarge sz = (nNibbles + 1) * sizeof(void *);

	BPlusTree::Stack stack = (BPlusTree::Stack)mem(0, sz);
	memcpy(stack, s, sz);

	ULarge iNibble = nNibbles;
	while(!stack[iNibble] && iNibble > 1){
		iNibble--;
	}

	
	void ** pChildren; ;
	while(iNibble > 0 && iNibble <= nNibbles && ((pChildren = (void **)*stack[iNibble - 1])!= 0)){
		SLarge iChild = (1 - iDirection) / 2 * (m_nChildrenPerNode -1);
		if(stack[iNibble]){
			iChild = (ULarge)(stack[iNibble] - pChildren) + iDirection;
		}
		while(iChild < m_nChildrenPerNode && iChild >= 0){
			if(pChildren[iChild]){
				stack[iNibble] = pChildren + iChild;

				break;
			}
			else{
				iChild += iDirection;
			}
		}

		if(iChild == m_nChildrenPerNode || iChild == -1){
			stack[iNibble] = 0;
			iNibble--;
		}
		else{
			iNibble++;
		}

	}


	if(iNibble == (nNibbles + 1)){
		bRet = true;
	}

	if(bRet){
		memcpy(s, stack, sz);
	}
	mem(stack, 0);
	return bRet;
}


void BPlusTree::path(Stack s, USmall & iKeyNibble, kq::core::ui8 * pKey){

	USmall nKeyNibbles = m_nNibblesInKey;
	USmall iInnerKeyNibble = m_nNibblesInKey - iKeyNibble - 1;
	kq::core::ui8 halfKey;


	while(s[iKeyNibble] && *s[iKeyNibble] && (iKeyNibble < nKeyNibbles)){

		USmall iByte = iInnerKeyNibble / m_nNibblesPerByte;
		USmall iNibbleID = iInnerKeyNibble % m_nNibblesPerByte;

		USmall & byte = pKey[iByte];
		USmall maskedByte = byte & m_pNibbleMasks[iNibbleID];
		halfKey = (maskedByte >> (m_nBitsPerLevel * iNibbleID));

		s[iKeyNibble + 1] = ((void**)*s[iKeyNibble]) + halfKey;
		iKeyNibble++;
		iInnerKeyNibble--;
	}
}

void BPlusTree::dumpStack(void *** pppCurr){

	ULarge nNibbles = m_nNibblesInKey;
	ULarge iNibble = 1;
	while(iNibble <= nNibbles){
		void ** pFirst = (void **)pppCurr[iNibble - 1][0];
		void ** pCurrent = pppCurr[iNibble];

		printf("%2X ", (int)(pCurrent - pFirst));

		iNibble++;
	}

	printf("%p\n", pppCurr[nNibbles][0]);
}


bool BPlusTree::Path::move(void ** v, void * k, SSmall iDirection){
	bool bRet = false;
	if(tree->nextleaf(s, iDirection)){
		current(v, k);
		bRet = true;
	}

	return bRet;
}


bool BPlusTree::Path::init(void ** v, void * k, SSmall iDirection){
	bool bRet = false;
	if(s){
		tree->stackDestroy(s);
		s = 0;
	}

	s = tree->stackCreate();

	if(s){
		if(iDirection){
			if(tree->nextleaf(s, iDirection)){
				current(v, k);
				bRet = true;
			}
		}
		else {
			if(tree->getStackFromKey(s, k)){
				bRet = true;
				if(v){
					*v = tree->readValueAtStack(s);
				}
			}
		}

		if(!bRet){
			tree->stackDestroy(s);
			s = 0;
		}
	}
	return bRet;
}

BPlusTree::Path::Path(BPlusTree * pTree): tree(pTree),s(0){
	
}

const BPlusTree::Path & BPlusTree::Path::operator =(const BPlusTree::Path & other){

	if(tree != other.tree){
		if(tree->m_nLevelsInStack != other.tree->m_nLevelsInStack){
			if(s){
				tree->stackDestroy(s);
				s = 0;
			}
			
		}
		tree = other.tree;

	}

	if(other.s){
		if(!s){
			s = tree->stackCreate();
		}
		if(s){
			tree->stackCopy(s, other.s);
		}
	}
	else{
		if(s){
			tree->stackDestroy(s);
			s = 0;
		}
	}

	return *this;

}

bool BPlusTree::Path::next(void ** v, void * k){
	return move(v, k, 1);
}

bool BPlusTree::Path::prev(void ** v, void * k){
	return move(v, k, -1);
}

bool BPlusTree::Path::init_first(void ** v, void * k){
	return init(v,k, 1);
}

bool BPlusTree::Path::init_last(void ** v, void * k){
	return init(v,k, -1);
}

bool BPlusTree::Path::init_moveTo(void * k, void ** v){
	return init(v, k, 0);
}

void BPlusTree::Path::current(void ** v, void * k){
	if(v){
		*v = tree->readValueAtStack(s);
	}
	if(k){
		tree->getKeyFromStack(k, s);
	}
}

void BPlusTree::Path::write(void * v){
	tree->writeValueAtStack(s, v);
};

BPlusTree::Path::~Path(){
	//printf("Path destroying\n");fflush(stdout);
	if(s){
		tree->stackDestroy(s);
		s = 0;
	}
	//printf("Path destroyed\n");fflush(stdout);
}


void BPlusTree::dump(){
	Path p(this);
	ui8 * k = (ui8 *)calloc(1, m_nBytesInKey);
	USmall iByte;
	void * v;
	if(p.init_first(&v, k)){
		do{
			for(iByte = 0; iByte < m_nBytesInKey; iByte++){
				printf("%.2x", (int)k[m_nBytesInKey - iByte - 1]);
			}
			printf("=%p\n", v);
		}while(p.next(&v, k));

	}
}




