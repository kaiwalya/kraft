/*
 * IVideoConverters.cpp
 *
 *  Created on: Mar 3, 2009
 *      Author: k
 */

#include "IVideoGrabber.hpp"
#include "oops/Exception.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

//for memcpy
#include "string.h"


using namespace dos::utils;
typedef IVideoGrabber::VideoBuffer VideoBuffer;

static void convert_null(VideoBuffer * pOut, VideoBuffer * pIn){
	memcpy(pOut->address, pIn->address, pOut->length);
}

static void convert_notImplemented(VideoBuffer * pOut, VideoBuffer * pIn){
	dos_log("The reqired conversion from %d to %d is not implemented", pIn->format, pIn->format);
	throw dos::oops::NotImplementedException("Connot Convert Formats");
}

static void convert_unknown(VideoBuffer * pOut, VideoBuffer * pIn){
	dos_log("Conversion From/To unknown format requested");
	throw dos::oops::BadProgrammingException("Connot Convert Formats");
}


static void convert_rgb_rgba(VideoBuffer * pOut, VideoBuffer * pIn){
	int i;

	unsigned char * pSrc = reinterpret_cast<unsigned char *>(pIn->address);
	unsigned char * pDest = reinterpret_cast<unsigned char *>(pOut->address);
	int iWidth = pOut->width;
	int iHeight = pOut->height;

	for(i = 0; i < iWidth * iHeight; i++){
		pDest[0] = pSrc[0];
		pDest[1] = pSrc[1];
		pDest[2] = pSrc[2];
		pDest[3] = 255;

		pDest += 4;
		pSrc += 3;
	}

}


static void convert_yuvu_rgba(VideoBuffer * pOut, VideoBuffer * pIn){
	int i;

	unsigned char * pSrc = reinterpret_cast<unsigned char *>(pIn->address);
	unsigned char * pDest = reinterpret_cast<unsigned char *>(pOut->address);
	int iWidth = pOut->width;
	int iHeight = pOut->height;

	for(i = 0; i < iWidth * iHeight/2 ; i++){
		float y0,u,v,y1;
		y0 =(pSrc[0]) * 1.0;
		u = (pSrc[1] - 128) * 1.0;
		y1 =(pSrc[2]) * 1.0;
		v = (pSrc[3] - 128) * 1.0;

		float rgba[8];
		/*

		rgba[0] = 1.164*(y0) + 1.596 * (v);
		rgba[1] = 1.164*(y0) - 0.391 * (u) - 0.813 * (v);
		rgba[2] = 1.164*(y0) + 2.018 * (u);

		rgba[4] = 1.164*(y1) + 1.596 * (v);
		rgba[5] = 1.164*(y1) - 0.391 * (u) - 0.813 * (v);
		rgba[6] = 1.164*(y1) + 2.018 * (u);
		 */



		rgba[0] = (y0) + 1.40200 * (v);
		rgba[1] = (y0) - 0.344136 * (u) - 0.714136 * (v);
		rgba[2] = (y0) + 1.77200 * (u);
		rgba[3] = 255;
		rgba[4] = (y1) + 1.40200 * (v);
		rgba[5] = (y1) - 0.344136 * (u) - 0.714136 * (v);
		rgba[6] = (y1) + 1.77200 * (u);
		rgba[7] = 255;


		int i;
		for(i = 0 ; i < 3; i++){
			if(rgba[i] > 255){
				pDest[i] = 255;
			}else{
				if(rgba[i] < 0){
					pDest[i] = 0;
				}
				pDest[i] = rgba[i];
			}

			if(rgba[i + 4] > 255){
				pDest[i + 4] = 255;
			}else{
				if(rgba[i + 4] < 0){
					pDest[i + 4] = 0;
				}
				pDest[i + 4] = rgba[i + 4];
			}
		}
		pDest += 8;
		pSrc += 4;
	}

}


void (*IVideoGrabber::s_convertfuncs[vf_MAX][vf_MAX])(VideoBuffer * pOut, VideoBuffer * pIn) =
{
		//In decides the row
		//Out decides the column
		{&convert_unknown,	&convert_unknown,	&convert_unknown,			&convert_unknown,},
		{&convert_unknown,	&convert_null,		&convert_notImplemented,	&convert_notImplemented,},
		{&convert_unknown,	&convert_yuvu_rgba,	&convert_null,				&convert_notImplemented,},
		{&convert_unknown,  &convert_rgb_rgba,  &convert_notImplemented,	&convert_null,}
};
