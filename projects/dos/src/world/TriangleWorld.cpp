/*
 * TriangleWorld.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: k
 */

#include "TriangleWorld.hpp"
#include "malloc.h"
#include "globals.hpp"
#include "system/Engine.hpp"
#include "utils/logger.hpp"
#include "oops/Exception.hpp"


using namespace dos::world;

TriangleWorld::TriangleWorld() {

	//m_texCam = 0;
	//m_iVideoFrameCount = -1;
	//m_pVideoFrameBuffer = 0;
}

TriangleWorld::~TriangleWorld() {

}

void TriangleWorld::updateVideoFrameBuffer(){

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_videoBuffer.width, m_videoBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_videoBuffer.address);

}

void TriangleWorld::up(void **){

	g_pGlobal->e->getVideoGrabber()->getFrame(&m_videoBuffer);
	m_iVideoFrameCount = m_videoBuffer.iFrameCount;


	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_texCam);
	glBindTexture(GL_TEXTURE_2D, m_texCam);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_videoBuffer.width, m_videoBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_videoBuffer.address);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

}

void TriangleWorld::down(void **){


	glDeleteTextures(1, &m_texCam);
	m_texCam = 0;


}

void TriangleWorld::process(void **){

}

void TriangleWorld::move(void **){

	g_pGlobal->e->getVideoGrabber()->getFrame(&m_videoBuffer);
	if( m_videoBuffer.iFrameCount > m_iVideoFrameCount && m_videoBuffer.address ){
		if(m_videoBuffer.format == dos::utils::VideoGrabber::vf_RGBA_32){
			updateVideoFrameBuffer();
			m_iVideoFrameCount = m_videoBuffer.iFrameCount;
		}
		else{
			dos_log("This format is not supposed to be here");
			throw oops::BadProgrammingException("This format is not supposed to be here");
		}
	}

}

void TriangleWorld::render(void **){

	glClear(GL_DEPTH_BUFFER_BIT);

	float fSize = 0.4;
	float fAspect = 1.0 * m_videoBuffer.width/m_videoBuffer.height;
	glBegin(GL_QUADS);

	glColor4d(1, 1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, m_texCam);
	glTexCoord2d(1, 0); glVertex3d(-fSize * fAspect, fSize, -0.9);
	glTexCoord2d(0, 0); glVertex3d(fSize*fAspect, fSize, -0.9);
	glTexCoord2d(0, 1); glVertex3d(fSize*fAspect, -fSize, -0.9);
	glTexCoord2d(1, 1); glVertex3d(-fSize*fAspect, -fSize, -0.9);


	glEnd();


}
