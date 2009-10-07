#include "IVideoGrabber.hpp"
#include "VideoGrabber.hpp"
#include "VideoGrabberV1.hpp"
#include "globals.hpp"
#include "utils/DynamicVariables.hpp"
#include "utils/logger.hpp"
#include "oops/Exception.hpp"
#include "stdlib.h"

using namespace dos::utils;

IVideoGrabber * IVideoGrabber::constructByTesting(){
	dos_log("Testing available drivers");
	IVideoGrabber *grabbers[] = {new VideoGrabberV1(), new VideoGrabber(), new NullGrabber(), };
	int iSuccessIndex = -1;

	int iGrabber, nGrabbers;
	nGrabbers = sizeof(grabbers)/sizeof(grabbers[0]);
	for(iGrabber = 0; iGrabber < nGrabbers; iGrabber++){
		try{
			VideoBuffer b = {0};
			grabbers[iGrabber]->up(0);
			grabbers[iGrabber]->getFrame(&b);
		}
		catch(oops::Exception E){
			E.diagnose(0);
			grabbers[iGrabber]->down(0);
			delete grabbers[iGrabber];
			grabbers[iGrabber] = 0;
		}

		//If the object was not deleted above,
		//no exception was raised and hence this works
		if(grabbers[iGrabber]){
			grabbers[iGrabber]->down(0);
			iSuccessIndex = iGrabber;
			break;
		}
	}

	if(iSuccessIndex >= 0){
		for(iGrabber = iSuccessIndex + 1; iGrabber < nGrabbers ; iGrabber++){
			delete grabbers[iGrabber];
			grabbers[iGrabber] = 0;
		}
	}else{
		grabbers[0] = new NullGrabber();
		iSuccessIndex = 0;
	}

	return grabbers[iSuccessIndex];
}

IVideoGrabber * IVideoGrabber::construct(){
	int iVidVer = -1;
	std::string * pVer;
	if((pVer = dos::g_pGlobal->vars->getString("vidver")) != 0){
		iVidVer = atoi(pVer->c_str());
		if(iVidVer > 2){
			iVidVer = 2;
		}
		if(iVidVer < 1){
			iVidVer = 1;
		}
	}

	IVideoGrabber * pRet = 0;

	switch(iVidVer){
	case 1:
		pRet = new VideoGrabberV1();
		break;
	case 2:
		pRet = new VideoGrabber();
		break;
	default:
		pRet = constructByTesting();
		break;
	}

	return pRet;


}

void IVideoGrabber::convertFormat(VideoBuffer * pOut, VideoBuffer * pIn){
	(*s_convertfuncs[pIn->format][pOut->format])(pOut, pIn);
}



void NullGrabber::up(void **){};
void NullGrabber::process(void **){}
void NullGrabber::down(void **){}
void NullGrabber::getFrame(VideoBuffer * pBuffer){
	pBuffer->address = 0;
	pBuffer->length = 0;
	pBuffer->iFrameCount = -1;
	pBuffer->height = 0;
	pBuffer->width = 0;
};
