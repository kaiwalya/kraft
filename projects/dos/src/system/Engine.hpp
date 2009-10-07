#ifndef SYSTEM_ENGINE_H_
#define SYSTEM_ENGINE_H_

#include "IWindowManager.hpp"
#include "ITimer.hpp"
#include "../world/IWorld.hpp"
#include "utils/IVideoGrabber.hpp"

namespace dos {

	namespace system {



		class Engine {
		protected:
			IWindowManager * m_pWindowManager;
			ITimer * m_pTimer;
			dos::world::IWorld * m_pCurrentWorld;
			dos::world::IWorld * m_pDefaultWorld;

			dos::utils::IVideoGrabber * m_pVideoGrabber;
		public:
			Engine();

			dos::utils::IVideoGrabber * getVideoGrabber();

			virtual void up(void **);
			virtual void down(void **);
			virtual void process(void **);

		};

	}

}

#endif /* ENGINE_H_ */
