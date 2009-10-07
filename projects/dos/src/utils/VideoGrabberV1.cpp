/*
 * VideoGrabberV1.cpp
 *
 *  Created on: Feb 27, 2009
 *      Author: k, Ankur
 */

#include "VideoGrabberV1.hpp"
/* Simple Video4Linux image grabber. */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/videodev.h>
#include <string.h>


#include "oops/Exception.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

#define ZERO(x) (memset(&x, 0, sizeof(x)))
#define ZEROMEM(x,y) (memset(x, 0, y))

/* Stole this from tvset.c */
/*
#define READ_VIDEO_PIXEL(buf, format, depth, r, g, b)			\
{									\
                                                                        \
        switch (format)							\
	{								\
		case VIDEO_PALETTE_GREY:				\
			switch (depth)					\
			{						\
				case 4:					\
				case 6:					\
				case 8:					\
					(r) = (g) = (b) = (*buf++ << 8);\
					break;				\
									\
				case 16:				\
					(r) = (g) = (b) = 		\
						*((unsigned short *) buf);	\
					buf += 2;			\
					break;				\
			}						\
			break;						\
									\
									\
		case VIDEO_PALETTE_RGB565:				\
		{							\
			unsigned short tmp = *(unsigned short *)buf;	\
			(r) = tmp&0xF800;				\
			(g) = (tmp<<5)&0xFC00;				\
			(b) = (tmp<<11)&0xF800;				\
			buf += 2;					\
		}							\
		break;							\
									\
		case VIDEO_PALETTE_RGB555:				\
			(r) = (buf[0]&0xF8)<<8;				\
			(g) = ((buf[0] << 5 | buf[1] >> 3)&0xF8)<<8;	\
			(b) = ((buf[1] << 2 ) & 0xF8)<<8;		\
			buf += 2;					\
			break;						\
									\
		case VIDEO_PALETTE_RGB24:				\
			(r) = buf[0] << 8; (g) = buf[1] << 8; 		\
			(b) = buf[2] << 8;				\
			buf += 3;					\
			break;						\
									\
		default:						\
			fprintf(stderr, 				\
				"Format %d not yet supported\n",	\
				format);				\
	}								\
}
*/

/*
int get_brightness_adj(unsigned char *image, long size, int *brightness) {
	long i, tot = 0;
	for (i = 0; i < size * 3; i++)
		tot += image[i];
	*brightness = (128 - tot / (size * 3)) / 3;
	return !((tot / (size * 3)) >= 126 && (tot / (size * 3)) <= 130);
}
*/

using namespace dos::utils;

VideoGrabberV1::VideoGrabberV1() {
	ZERO(m_inBuffer);
	ZERO(m_outBuffer);
	m_iCameraFile = 0;

}

VideoGrabberV1::~VideoGrabberV1() {

}

void VideoGrabberV1::up(void **) {
	if(m_outBuffer.address || m_inBuffer.address || m_iCameraFile){
		throw dos::oops::BadProgrammingException("up called twice, w/o call to down");
	}

	int iRet;
	iRet = open("/dev/video0", O_RDONLY);
	if(iRet < 0){
		throw oops::APIFailureException("Error opening \"/dev/video0\"");
	}
	m_iCameraFile = iRet;



	//Get Capabilities
	video_capability cap;ZERO(cap);
	video_window win;ZERO(win);
	video_picture vpic;ZERO(vpic);
	{


		if (ioctl(m_iCameraFile, VIDIOCGCAP, &cap) < 0) {
			throw oops::APIFailureException("Error querying video device capabilities:VIDIOCGCAP");
		}

		if (ioctl(m_iCameraFile, VIDIOCGWIN, &win) < 0) {
			throw oops::APIFailureException("Error querying video device capabilities:VIDIOCGWIN");
		}

		if (ioctl(m_iCameraFile, VIDIOCGPICT, &vpic) < 0) {
			throw oops::APIFailureException("Error querying video device capabilities:VIDIOCGPICT");
		}


		dos_log("Camera: %s", cap.name);
	}


	if (cap.type & VID_TYPE_MONOCHROME) {
		throw oops::NotImplementedException("Camera Type Monochrome not programmed");
	}

	//Try setting 24 bit color

	vpic.depth = 24;
	vpic.palette = VIDEO_PALETTE_RGB24;
	iRet = ioctl(m_iCameraFile, VIDIOCSPICT, &vpic);
	m_inBuffer.format = vf_RGB_24;
	if(iRet < 0){

		vpic.depth = 32;
		vpic.palette = VIDEO_PALETTE_RGB32;
		iRet = ioctl(m_iCameraFile, VIDIOCSPICT, &vpic);
		m_inBuffer.format = vf_RGBA_32;
		if(iRet < 0){

			vpic.depth = 32;
			vpic.palette = VIDEO_PALETTE_YUYV;
			iRet = ioctl(m_iCameraFile, VIDIOCSPICT, &vpic);
			m_inBuffer.format = vf_YUVU_32;
			if(iRet < 0){
				dos_log("No RGB24/RGB32/YUVU Support");
				throw dos::oops::NotImplementedException("Couldnot find supported format");
			}
		}
	}


	dos_log("%dbit, %dx%d", vpic.depth, win.width, win.height);

	m_iInBPP = vpic.depth;
	m_inBuffer.length = win.width * win.height * vpic.depth;
	m_outBuffer.length = win.width * win.height * 32;
	void * pBuffIn = malloc(m_inBuffer.length);
	void * pBuffOut = malloc(m_outBuffer.length);
	if(!pBuffIn || !pBuffOut){
		dos_log("No mem");
		throw dos::oops::APIFailureException("Malloc Failed");
	}

	m_inBuffer.address = reinterpret_cast<unsigned char *>(pBuffIn);
	m_outBuffer.address = reinterpret_cast<unsigned char *>(pBuffOut);
	m_outBuffer.format = vf_RGBA_32;

	m_inBuffer.width = m_outBuffer.width = win.width;
	m_inBuffer.height = m_outBuffer.height = win.height;

	m_inBuffer.iFrameCount = -1;
	m_outBuffer.iFrameCount = -1;

}

void VideoGrabberV1::down(void **) {

	free(m_inBuffer.address);
	ZERO(m_inBuffer);
	free(m_outBuffer.address);
	ZERO(m_outBuffer);

	close(m_iCameraFile);
	m_iCameraFile = 0;

	dos_log("Camera Closed");

}

void VideoGrabberV1::process(void **) {

}

void VideoGrabberV1::getFrame(VideoBuffer * pBuffer) {
	if(!pBuffer){
		return;
	}
	int iRet = 0;
	iRet = read(m_iCameraFile, m_inBuffer.address, m_inBuffer.width * m_inBuffer.height * m_iInBPP);
	if(iRet < 0){
		dos_log("Camera read error");
		throw dos::oops::APIFailureException("Camera Read Failed");
	}else{
		m_inBuffer.iFrameCount++;
		convertFormat(&m_outBuffer, &m_inBuffer);
		m_outBuffer.iFrameCount++;
	}

	*pBuffer = m_outBuffer;
}
