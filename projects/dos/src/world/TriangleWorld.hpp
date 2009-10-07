#ifndef TRIANGLEWORLD_H_
#define TRIANGLEWORLD_H_
#include "IWorld.hpp"
#include "GL/gl.h"
#include "GL/glu.h"
#include "utils/VideoGrabber.hpp"

namespace dos {
	namespace world {
		class TriangleWorld: public dos::world::IWorld {
		public:
			TriangleWorld();
			virtual ~TriangleWorld();
			GLuint m_texCam;
			dos::utils::VideoGrabber::VideoBuffer m_videoBuffer;
			int m_iVideoFrameCount;
			void updateVideoFrameBuffer();
		public:
			virtual void up(void **);
			virtual void down(void **);
			virtual void process(void **);

			virtual void move(void **);
			virtual void render(void **);
		};
	}
}

#endif /* TRIANGLEWORLD_H_ */
