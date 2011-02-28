#include "core_data_BPlusTree.hpp"


#include "stdio.h"
#include "stdlib.h"
#include "string.h"

using namespace kq;
using namespace kq::core;
using namespace kq::core::data;

void BPlusTree::path(void *** pppCurr, USmall & iKeyNibble, kq::core::ui8 * pKey){

	USmall nKeyNibbles = nBytesInKey * nNibblesPerByte;
	kq::core::ui8 halfKey;


	while(pppCurr[iKeyNibble] && *pppCurr[iKeyNibble] && (iKeyNibble < nKeyNibbles)){

		USmall iByte = iKeyNibble / nNibblesPerByte;
		USmall iNibbleID = iKeyNibble % nNibblesPerByte;

		USmall & byte = pKey[iByte];
		USmall maskedByte = byte & pNibbleMasks[iNibbleID];
		halfKey = (maskedByte >> (nBitsPerLevel * iNibbleID));

		pppCurr[iKeyNibble + 1] = ((void**)*pppCurr[iKeyNibble]) + halfKey;
		iKeyNibble++;

		
	}

}

void BPlusTree::move(void *** pppCurr, USmall & iKeyNibble, kq::core::ui8 * pKey){

	USmall nKeyNibbles = nBytesInKey * nNibblesPerByte;
	kq::core::ui8 halfKey;

	
	while(**pppCurr && (iKeyNibble < nKeyNibbles)){

		USmall iByte = iKeyNibble / nNibblesPerByte;
		USmall iNibbleID = iKeyNibble % nNibblesPerByte;

		USmall & byte = pKey[iByte];
		USmall maskedByte = byte & pNibbleMasks[iNibbleID];
		halfKey = (maskedByte >> (nBitsPerLevel * iNibbleID));
		*pppCurr = ((void**)**pppCurr) + halfKey;
	
		iKeyNibble++;
	}

	
}


bool BPlusTree::nextleaf(BPlusTree::Stack s, SSmall iDirection){
	bool bRet = false;
	ULarge nNibbles = nBytesInKey * nNibblesPerByte;
	ULarge sz = (nNibbles + 1) * sizeof(void *);

	BPlusTree::Stack stack = (BPlusTree::Stack)calloc(1, sz);
	memcpy(stack, s, sz);

	ULarge iNibble = nNibbles;
	while(!stack[iNibble] && iNibble > 1){
		iNibble--;
	}

	while(iNibble > 0 && iNibble <= nNibbles){
		void ** pChildren = (void **)*stack[iNibble - 1];
		SLarge iChild = (1 - iDirection) / 2 * (nChildrenPerNode -1);
		if(stack[iNibble]){
			iChild = (ULarge)(stack[iNibble] - pChildren) + iDirection;
		}
		while(iChild < nChildrenPerNode && iChild >= 0){
			if(pChildren[iChild]){
				stack[iNibble] = pChildren + iChild;

				break;
			}
			else{
				iChild += iDirection;
			}
		}

		if(iChild == nChildrenPerNode || iChild == -1){
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
	return bRet;
}


void BPlusTree::dumpStack(void *** pppCurr){
	
	ULarge nNibbles = nBytesInKey * nNibblesPerByte;
	ULarge iNibble = 1;
	while(iNibble <= nNibbles){
		void ** pFirst = (void **)pppCurr[iNibble - 1][0];
		void ** pCurrent = pppCurr[iNibble];

		printf("%2X ", (int)(pCurrent - pFirst));

		iNibble++;
	}

	printf("%p\n", pppCurr[nNibbles][0]);
}

BPlusTree::BPlusTree(kq::core::memory::MemoryWorker memworker, USmall bytesInKey, USmall bitsPerLevel)
	:mem(memworker),
	nBytesInKey(bytesInKey),
	nBitsPerLevel(bitsPerLevel),
	nChildrenPerNode(1 << bitsPerLevel),
	nNibblesPerByte(8/bitsPerLevel),
	nNibblesInKey(nBytesInKey * nNibblesPerByte),
	nLevelsInStack(nNibblesInKey + 1),
	nSizeOfStack(nLevelsInStack * sizeof(void *)),
	pRoot(0)
{

	//LOGINOUT;
	pNibbleMasks = (USmall *)mem(0, sizeof(USmall) * nNibblesPerByte);
	{
		USmall iBit = 0;
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


void ** BPlusTree::find(void * k){

	//LOGINOUT;
	kq::core::ui8 * pKey = (kq::core::ui8 *)k;
	USmall iKeyNibble = 0;
	USmall nKeyNibbles = nBytesInKey * nNibblesPerByte;
	

	void ** ppRet = 0;

	//Try to find store the found node in pCurr and nibbles left in iKeyNibble
	void ** ppCurr = &pRoot;
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
	USmall nKeyNibbles = nBytesInKey * nNibblesPerByte;
	

	void ** ppRet = 0;

	//Try to find store the found node in pCurr and nibbles left in iKeyNibble
	void ** ppCurr = &pRoot;
	move(&ppCurr, iKeyNibble, pKey);

	if(iKeyNibble < nKeyNibbles){			
		ULarge nSizeLevel = sizeof(void *) * nChildrenPerNode;

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

bool BPlusTree::destroy(void * pKey, void ** oldValue){
	//LOGINOUT;
	bool bRet = false;
	USmall nNibbles = nBytesInKey * nNibblesPerByte;
	USmall iNibble = 0;

	void *** pppPath = (void ***)mem(0, (nNibbles + 1) * sizeof(void **));
	if(pppPath){
		pppPath[iNibble] = &pRoot;
		path(pppPath, iNibble, (kq::core::ui8 *)pKey);
		if(iNibble == nNibbles){
			if(oldValue){
				*oldValue = *pppPath[nNibbles];
			}
			*pppPath[nNibbles] = 0;

			bRet = true;
			ULarge iChild;
			ULarge nDeadChildren;

			while(iNibble > 0){
				iChild = 0;
				nDeadChildren = 0;
				void ** ppFirstChild = (void **)*pppPath[iNibble-1];

				while(iChild < nChildrenPerNode){
					if(!ppFirstChild[iChild]){
						nDeadChildren++;
						iChild++;
						continue;
					}
					break;
				}
				if(nDeadChildren != nChildrenPerNode){
					break;
				}
				//LOGDEPTH;
				//printf("*(%p) [%p -> 0]\n", pppPath[iNibble-1], *pppPath[iNibble-1]);
				mem(ppFirstChild, 0);
				*pppPath[iNibble-1] = 0;

				iNibble--;
			}
		}
		mem(pppPath, 0);
	}

	return bRet;
}

BPlusTree::Stack BPlusTree::createStack(){
	const ULarge sz = sizeof(void *) * nBytesInKey * nNibblesPerByte;
	BPlusTree::Stack s = (BPlusTree::Stack)mem(0, sz);
	if(s){
		memset(s + 1, 0, sz);
		s[0] = &pRoot;
	}
	return s;
}
void BPlusTree::destroyStack(BPlusTree::Stack s){
	mem(s, 0);
}



bool BPlusTree::getStackFromKey(BPlusTree::Stack s, void * k){
	const ULarge nNibbles = nBytesInKey * nNibblesPerByte;
	USmall iNibble = 0;
	bool bRet;
	BPlusTree::Stack stack = createStack();
	if(stack){
		path(stack, iNibble, (kq::core::ui8 *)k);
		if(iNibble == nNibbles){
			bRet = true;
			memcpy(s, stack, nSizeOfStack);
		}
		destroyStack(stack);
	}
	return bRet;
}

void BPlusTree::getKeyFromStack(void * k, BPlusTree::Stack s){

	USmall iNibble = 0;

	kq::core::ui8 * key = (kq::core::ui8 *)k;
	memset(key, 0, nBytesInKey);
	while(iNibble < nNibblesInKey && s[iNibble + 1]){

		USmall iByte = (USmall)(iNibble / nNibblesPerByte);
		USmall iNID = iNibble % nNibblesPerByte;
		USmall &byte = key[iByte];


		USmall halfKey = (USmall)(s[iNibble + 1] - (void **)*s[iNibble]);
		USmall maskedByte = halfKey << (nBitsPerLevel * iNID);
		byte = byte | maskedByte;
		iNibble++;
	}

};

void * BPlusTree::readValueAtStack(BPlusTree::Stack s){
	const ULarge nNibbles = nBytesInKey * nNibblesPerByte;
	return *s[nNibbles];
}

void BPlusTree::writeValueAtStack(BPlusTree::Stack s, void * v){
	const ULarge nNibbles = nBytesInKey * nNibblesPerByte;
	*s[nNibbles] = v;
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
		}
	}
	else{
		if(oldV){
			void ** pLocation = find(key);
			if(pLocation){
				*oldV = *pLocation;
			}
		}
		destroy(key);
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

void BPlusTree::dump(){
	BPlusTree::Stack s = createStack();
	if(s){
		while(nextleaf(s)){
			dumpStack(s);
		}
		destroyStack(s);
	}
}



BPlusTree::~BPlusTree(){
	//LOGINOUT;
	mem(pNibbleMasks, 0);
	kq::core::ui64 iLeftForDead = 0;
	
	if(nBytesInKey){
		USmall nKeyNibbles = nBytesInKey * nNibblesPerByte;
		
		void ** stackNode = (void**)mem(0, nKeyNibbles * sizeof(void **));
		ULarge * stackIndex = (ULarge *)mem(0, nKeyNibbles * sizeof(ULarge));
		USmall top = nKeyNibbles - 1;

		stackNode[top] = pRoot;
		stackIndex[top] = 0;

		void ** pCurr;		
		while((pCurr = (void**)stackNode[top]) != 0){			
			ULarge iCurrChild = stackIndex[top];

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
}


/*
bool move(void ** v, void * k, SSmall iDirection){			
	bool bRet = false;
	if(tree->nextleaf(s, iDirection)){
		current(v, k);
		bRet = true;
	}

	return bRet;
}

bool init(void ** v, void * k, SSmall iDirection){
	bool bRet = false;
	if(s){
		tree->destroyStack(s);
		s = 0;
	}

	s = tree->createStack();

	if(s){
		if(tree->nextleaf(s, iDirection)){
			current(v, k);
			bRet = true;
		}
		else{
			tree->destroyStack(s);
		}
	}
	return bRet;
}

public:
path(BPlusTree * pTree): tree(pTree){
	
}

bool next(void ** v, void * k){
	return move(v, k, 1);
}

bool prev(void ** v, void * k){
	return move(v, k, -1);
}

bool init_first(void ** v, void * k){
	return init(v,k, 1);
}

bool init_last(void ** v, void * k){
	return init(v,k, -1);
}

void current(void ** v, void * k){
	if(v){
		tree->readValueAtStack(s);
	}
	if(k){
		tree->getKeyFromStack(k, s);
	}
}

bool moveTo(void * k, void ** v){
	bool bRet = false;
	if(tree->getStackFromKey(s, k)){
		bRet = true;
		current(v, k);
	}
	return true;
}

void write(void * v){
	tree->writeValueAtStack(s, v);
};

~path(){
	if(s){
		tree->destroyStack(s);
	}
}



*/