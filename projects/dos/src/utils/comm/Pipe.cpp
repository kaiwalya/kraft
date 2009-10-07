/*
 * Pipe.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: k
 */

#include "Pipe.hpp"
#include "malloc.h"

using namespace dos::utils::comm;

LoopbackPipe::LoopbackPipe(){
	m_pHead = 0;
	m_pTail = 0;
}

LoopbackPipe::~LoopbackPipe(){
	Fragment f;

	pop(f);
	while(f.size){
		free(reinterpret_cast<void *>(f.location));
		pop(f);
	}
}

void LoopbackPipe::pop(Fragment & f){
	if(!m_pHead){
		f.size = 0;
		return;
	}

	f.size = m_pHead->size;
	f.location = m_pHead->location;

	Fragment * pNextHead = reinterpret_cast<Fragment *>(m_pHead->next);

	delete m_pHead;
	if(m_pHead == m_pTail){
		m_pTail = 0;
	}
	m_pHead = pNextHead;
}

void LoopbackPipe::push(Fragment * pFragment){

	//Check if its an unnecessary call
	if(!pFragment){
		return;
	}

	int iTotalSize = pFragment->getAggregateSize();
	if(iTotalSize <= 0){
		return;
	}

	Fragment * pDest = new Fragment;
	pDest->size = iTotalSize;
	void * pData = malloc(pDest->size);
	pFragment->getAggregateData(pData);
	pDest->location = reinterpret_cast<Location>(pData);
	pDest->next = 0;

	//Check if currently empty
	if(!m_pTail){
		//Ok create the first fragment
		m_pTail = m_pHead = pDest;
	}else{
		m_pTail->next = reinterpret_cast<Location>(pDest);
	}


}
