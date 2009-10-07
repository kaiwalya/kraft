/*
 * IPipe.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: k
 */


#include "IPipe.hpp"
#include "string.h"

using namespace dos::utils::comm;

IPipe::~IPipe(){

}

void Fragment::getAggregateData(void * pData){
	int iUnwrittenOffset = 0;
	Fragment * pFragment = this;
	char * pDest = reinterpret_cast<char *>(pData);
	while(pFragment){

		memcpy(pDest + iUnwrittenOffset, reinterpret_cast<void *>(pFragment->location), pFragment->size);
		iUnwrittenOffset += pFragment->size;

		pFragment = reinterpret_cast<Fragment *>(pFragment->next);
	}

}

Size Fragment::getAggregateSize(){
	int iRet = 0;
	Fragment * pFragment = this;
	while(pFragment){
		iRet += pFragment->size;
		pFragment = reinterpret_cast<Fragment *>(pFragment->next);
	}

	return iRet;
}


