/*
 * VideoGrabberV1.h
 *
 *  Created on: Feb 27, 2009
 *      Author: k
 */

#ifndef VIDEOGRABBERV1_H_
#define VIDEOGRABBERV1_H_

#include "IVideoGrabber.hpp"

namespace dos {

	namespace utils {

		class VideoGrabberV1: public dos::utils::IVideoGrabber {
		protected:
			int m_iCameraFile;
			int m_iInBPP;
			VideoBuffer m_inBuffer;
			VideoBuffer m_outBuffer;
		public:
			VideoGrabberV1();
			virtual ~VideoGrabberV1();
		public:
			void up(void **);
			void down(void **);
			void process(void **);
			void getFrame(VideoBuffer *);
		};

	}

}

#endif /* VIDEOGRABBERV1_H_ */
