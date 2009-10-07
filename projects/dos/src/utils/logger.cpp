#include "stdio.h"
#include "stdarg.h"
#include "string.h"

#include "logger.hpp"

using namespace dos::utils;

Logger::Logger(){
	m_pfnFilter = &defaultFilter;
}

bool Logger::defaultFilter(unsigned int iLevel){
	return false;
}

void Logger::log(unsigned int iLevel, const char * pFunc, const char * pSrc, const int iLine, const char * pFormat, ...){
	if(iLevel && (*m_pfnFilter)(iLevel)){
		return;
	}
	va_list params;
	va_start(params, pFormat);

	const int nMaxCharsInFunctionName = 20;
	char functionname[nMaxCharsInFunctionName + 1];

	const char * pFileName = pSrc;
	//Get the important part of the file name
	{
		const char sSrc[] = "/src";
		const int lSrc = *(int *)sSrc;

		if(pSrc[0] && pSrc[1] && pSrc[2]){
			while(pFileName[3] != 0){
				int lCurr = *(int *)pFileName;
				if(lCurr == lSrc){
					pFileName += 4;
					break;
				}
				pFileName++;
			}
		}
	}

	//Get the important part of the file name
	{


		int iFunctionNameLength = strlen(pFunc);
		const char * pFuncNameStart = pFunc;
		const char * pFuncNameEnd = pFunc + iFunctionNameLength;
		int nCharsRemoved = 0;
		while(pFuncNameEnd > pFuncNameStart){
			if(*pFuncNameEnd == '('){
				break;
			}
			pFuncNameEnd--;
			nCharsRemoved++;
		};

		int iFunctionNameLengthLeft = (iFunctionNameLength - nCharsRemoved);
		if(iFunctionNameLengthLeft > nMaxCharsInFunctionName){
			iFunctionNameLengthLeft = nMaxCharsInFunctionName;
		}


		memset(functionname, ' ', sizeof(functionname));
		memcpy(functionname, pFuncNameEnd - iFunctionNameLengthLeft, iFunctionNameLengthLeft);
		functionname[nMaxCharsInFunctionName] = 0;


		pFunc = functionname;

	}

	//fprintf(stderr,"(%s, %d)%s==>", pFileName, iLine, pFunc);


	fprintf(stderr,"%s @ %4d ==> ", pFunc, iLine);
	vfprintf(stderr, pFormat, params);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(params);
}


