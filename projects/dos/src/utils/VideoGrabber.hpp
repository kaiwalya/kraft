/*
 * VideoGrabber.h
 *
 *  Created on: Feb 20, 2009
 *      Author: k
 */

#ifndef VIDEOGRABBER_H_
#define VIDEOGRABBER_H_

#include "IVideoGrabber.hpp"

namespace dos {

	namespace utils {

		class VideoGrabber:public IVideoGrabber {


			bool m_bInitStreaming;
			bool m_bInitBlocking;
			void q();
			void dq();
		public:


			int m_iCameraFile;
			int m_nBuffers;
			VideoBuffer * m_pBuffers;
			int m_nBuffersInQueue;
			int m_iLastBufferQueued;
			int m_iLastBufferDeQueued;
			VideoBuffer m_outBuffer;
			void updateOutputBuffer();

		public:

			VideoGrabber();
			virtual ~VideoGrabber();

			virtual void up(void **);
			virtual void down(void **);
			virtual void process(void **);

			void getFrame(VideoBuffer * pFrame);
		};

	}

}

#endif /* VIDEOGRABBER_H_ */
