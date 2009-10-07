/*
 * VideoGrabber.cpp
 *
 *  Created on: Feb 20, 2009
 *      Author: k
 */

#include "VideoGrabber.hpp"

#include "fcntl.h"
#include "sys/ioctl.h"
#include "sys/errno.h"
#include "linux/videodev2.h"
#include "sys/mman.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "memory.h"

#include "oops/Exception.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

using namespace dos::utils;

VideoGrabber::VideoGrabber() {

}

VideoGrabber::~VideoGrabber() {

}

void VideoGrabber::up(void **){



	//Open Video Device
	int iRet;
	iRet = open("/dev/video0", O_RDONLY);
	if(iRet <= 0){
		throw oops::APIFailureException("Error opening \"/dev/video0\"");
	}
	m_iCameraFile = iRet;


	//Check if its a capture device
	{
		v4l2_capability c;
		memset(&c, 0, sizeof(c));
		iRet = ioctl(m_iCameraFile, VIDIOC_QUERYCAP, &c);
		if(iRet != 0){
			throw oops::APIFailureException("Error querying video device capabilities");

		}
		dos_log("Camera: %s|%s|Capture is %ssupported", c.card, c.driver, (c.capabilities & VID_TYPE_CAPTURE)?"":"not ");
		//If it is not stop
		if(!(c.capabilities & VID_TYPE_CAPTURE)){
			throw oops::MissingCapabilityException("Compatible Camera Not Found");
		}
	}

	//Select the default input
	{

		//(ioctl(iFile, VIDIOC_ENUMINPUT, &input) >= 0)

		int iInput = 0;
		iRet = ioctl(m_iCameraFile, VIDIOC_S_INPUT, &iInput);
		if(iRet != 0){
			throw oops::APIFailureException("Couldnt select default camera input");
		}

		v4l2_input input = {0};
		iRet = ioctl(m_iCameraFile, VIDIOC_ENUMINPUT, &input);
		if(iRet != 0){
			throw oops::APIFailureException("Couldnt enumerate current input");
		}

		dos_log("Input %d-%s, selected", input.index, input.name);

	}

	//Fix Capture Format
	{

		//Get current format
		v4l2_format fmt;
		memset(&fmt.fmt, 0, sizeof(fmt.fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		iRet = ioctl(m_iCameraFile, VIDIOC_G_FMT, &fmt);
		if(iRet != 0){
			throw oops::APIFailureException("Couldnt Get current input format");
		}

		m_outBuffer.width = fmt.fmt.pix.width;
		m_outBuffer.height = fmt.fmt.pix.height;

		//Try to switch to 640x480
		fmt.fmt.pix.width = 640;
		fmt.fmt.pix.height = 480;
		iRet = ioctl(m_iCameraFile, VIDIOC_S_FMT, &fmt);
		if(iRet != 0){
			dos_log("640x480 not supported, falling back to %dx%d", m_outBuffer.width, m_outBuffer.height);
		}

 		//Get current format
		iRet = ioctl(m_iCameraFile, VIDIOC_G_FMT, &fmt);
		if(iRet != 0){
			throw oops::APIFailureException("Couldnt Get current input format");
		}

		m_outBuffer.width = fmt.fmt.pix.width;
		m_outBuffer.height = fmt.fmt.pix.height;
		dos_log("Camera Resolution %dx%d", m_outBuffer.width, m_outBuffer.height);

		//Try changing the pixel format to V4L2_PIX_FMT_YUYV
		if(fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV){
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
			iRet = ioctl(m_iCameraFile, VIDIOC_S_FMT, &fmt);
			if(iRet != 0){
				throw oops::MissingCapabilityException("Camera doesnot support YUYV format");
			}
		}

		//Get current format make sure chages we made are really set
		iRet = ioctl(m_iCameraFile, VIDIOC_G_FMT, &fmt);
		if(iRet != 0){
			throw oops::APIFailureException("Couldnt Get current input format");
		}
		if(fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV){
			throw oops::MissingCapabilityException("Camera doesnot support YUYV format");
		}

		m_outBuffer.format = vf_YUVU_32;


	}

	//Create capture buffers
	{

		v4l2_requestbuffers reqbuf = {0};

		//Request maximum buffers and allocate structure
		{
			reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			reqbuf.memory = V4L2_MEMORY_MMAP;
			reqbuf.count = 5;
			iRet = ioctl(m_iCameraFile, VIDIOC_REQBUFS, &reqbuf);
			if(iRet != 0){
				throw oops::MissingCapabilityException("Camera doesnot support memory mapped streaming buffers");
			}
			dos_log("Memory Mapped Streaming Supported: %d internal buffers allocated", reqbuf.count);

			if(!reqbuf.count){
				throw oops::MissingCapabilityException("No buffers found.");
			}

			m_pBuffers = (VideoBuffer *)malloc(sizeof(VideoBuffer) * reqbuf.count);
			if(!m_pBuffers){
				throw oops::APIFailureException("Malloc Failed!!");
			}
			memset(m_pBuffers, 0, sizeof(VideoBuffer) * reqbuf.count);
			m_nBuffers = reqbuf.count;
		}

		//Map buffers to our space;
		{
			int iBuffer;
			for(iBuffer = 0; iBuffer < (int)reqbuf.count; iBuffer++){
				v4l2_buffer buff = {0};
				buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buff.memory = V4L2_MEMORY_MMAP;
				buff.index = iBuffer;
				iRet = ioctl(m_iCameraFile, VIDIOC_QUERYBUF, &buff);
				if(iRet != 0){
					throw oops::APIFailureException("Counld not Querry Buffer\n");
				}

				VideoBuffer * pVBuffer = m_pBuffers + iBuffer;

				pVBuffer->address = mmap(0, buff.length, PROT_READ , MAP_SHARED, m_iCameraFile, buff.m.offset);
				if(!pVBuffer->address){
					throw oops::APIFailureException("mmap failure, couldnt map internal buffers to our virtual memory.");
				}
				//dos_log("Buffer %2d, size %d mapped to %p", iBuffer, buff.length, pVBuffer->address);
				pVBuffer->length = buff.length;
				pVBuffer->width = m_outBuffer.width;
				pVBuffer->height = m_outBuffer.height;
				pVBuffer->format = m_outBuffer.format;


			}
			m_nBuffersInQueue = 0;
			m_iLastBufferQueued = -1;
			m_iLastBufferDeQueued = -1;
			m_outBuffer.format = vf_RGBA_32;

		}

		//Create the output buffer
		{
			void * pBuff = reinterpret_cast<void *>(new char[m_outBuffer.height *m_outBuffer.width *4]);
			m_outBuffer.address = pBuff;
		}

	}

	//start the streaming
	{
		int iStreamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		iRet = 0;
		iRet = ioctl(m_iCameraFile, VIDIOC_STREAMON, &iStreamType);
		if(iRet != 0){
			throw oops::APIFailureException("Could not start camera straming");
		}

		m_bInitStreaming = true;
	}

	//Make the io non blocking so that we can deal with mutiple buffers in a single thread
	{
		int iNB = 1;
		iRet = ioctl(m_iCameraFile, FIONBIO, &iNB);
		if(iRet != 0){
			throw oops::APIFailureException("Could not put camera into NONBLOCKING mode.");
		}
		m_bInitBlocking = true;
	}


	dos_log("Camera Is On!!");

}

void VideoGrabber::down(void **){
	dos_log("Downing Camera");
	int iRet;
	//Putback file into blocking mode
	if(m_bInitBlocking){
		int iNB = 0;
		iRet = ioctl(m_iCameraFile, FIONBIO, &iNB);
		if(iRet != 0){
			dos_log("Could not put camera into BLOCKING mode.");
		}
		m_bInitBlocking = 0;
	}

	if(m_bInitStreaming){
		int iStreamType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		iRet = ioctl(m_iCameraFile, VIDIOC_STREAMOFF, &iStreamType);
		if(iRet != 0){
			dos_log("Could not stop camera straming");
		}
		m_bInitStreaming = 0;
	}

	//unmap buffers
	if(m_pBuffers){

		int iBuffer;
		for(iBuffer = m_nBuffers - 1; iBuffer >= 0; iBuffer--){
			if(m_pBuffers[iBuffer].address){
				munmap(m_pBuffers[iBuffer].address, m_pBuffers[iBuffer].length);
			}
		}

		free(m_pBuffers);
		m_pBuffers = 0;
		m_nBuffers = 0;

	}

	if(m_outBuffer.address){
		delete [] reinterpret_cast<char *>(m_outBuffer.address);
		m_outBuffer.address = 0;
	}

	if(m_iCameraFile){
		close(m_iCameraFile);
		m_iCameraFile = 0;
	}
}
void VideoGrabber::process(void **){

	dq();
	q();

}

void VideoGrabber::updateOutputBuffer(){
	m_outBuffer.iFrameCount = m_iLastBufferDeQueued;

	if(m_iLastBufferDeQueued >= 0){
		VideoBuffer * pCurrent = &(m_pBuffers[m_iLastBufferDeQueued % m_nBuffers]);
		convertFormat(&m_outBuffer, pCurrent);
		m_outBuffer.iFrameCount = m_iLastBufferDeQueued;
	}
}

void VideoGrabber::getFrame(VideoBuffer * pBuffer){
	updateOutputBuffer();
	*pBuffer = m_outBuffer;
	/*
	if(m_iLastBufferDeQueued >=  0){



	}
	*/
	/*
	pBuffer->address = 0;
	pBuffer->length = 0;
	pBuffer->width = m_iFrameWidth;
	pBuffer->height = m_iFrameHeight;
	pBuffer->format = vf_YUVU_32;
	pBuffer->iFrameCount = 0;
	*/
}

void VideoGrabber::q(){

	while(m_nBuffersInQueue < m_nBuffers){
		int iRet;
		v4l2_buffer buff;
		buff.index = (m_iLastBufferQueued + 1)%m_nBuffers;
		buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buff.memory = V4L2_MEMORY_MMAP;

		iRet = ioctl(m_iCameraFile, VIDIOC_QBUF, &buff);
		if(iRet != 0 && errno != EAGAIN){
			dos_log("Counldnot queue buffer, err %d - 0x%x", errno, errno);
			throw oops::APIFailureException("Counldnot queue buffer");
		}

		m_iLastBufferQueued++;
		m_nBuffersInQueue++;
		//dos_log("%d queued", m_nBuffersInQueue);
	}

}
void VideoGrabber::dq(){
	while(m_nBuffersInQueue > 0){


		int iRet;
		v4l2_buffer buff;
		buff.index = (m_iLastBufferDeQueued + 1)%m_nBuffers;
		buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buff.memory = V4L2_MEMORY_MMAP;
		iRet = ioctl(m_iCameraFile, VIDIOC_DQBUF, &buff);

		if(iRet != 0){
			if(errno != EAGAIN){
				throw oops::APIFailureException("Counldnot dequeue buffer");
			}
			//Could not deque due to wait state
			//So last buffer dequed is our best bet;
			break;
		}

		//Success;
		m_iLastBufferDeQueued++;
		m_nBuffersInQueue--;
		//dos_log("%d in queue", m_nBuffersInQueue);

	}
}
