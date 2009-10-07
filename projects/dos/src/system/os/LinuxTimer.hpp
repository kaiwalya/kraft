#ifndef SYSTEM_LINUX_TIMER_H_
#define SYSTEM_LINUX_TIMER_H_

#include "../ITimer.hpp"
#include "sys/time.h"

namespace dos{
	namespace system{
		namespace os{
			class LinuxTimer:public dos::system::ITimer{
			protected:
				timeval m_tv;
				float fDelta;

				void _capture(timeval * pTV) const;
				float _getDelta(timeval * pTV) const;
				void _update(timeval * pTV);

			public:
				LinuxTimer();
				void checkpoint();
				float getCurrentDelta() const;
				float getDelta() const;
				const float * getDeltaAddress() const;
			};
		}
	}
}

#endif /* TIMER_H_ */
