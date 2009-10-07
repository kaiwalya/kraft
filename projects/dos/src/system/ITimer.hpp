#ifndef ITIMER_H_
#define ITIMER_H_

namespace dos {

	namespace system {

		class ITimer {
		protected:
			virtual void checkpoint() = 0;

			friend class Engine;
		public:

			virtual ~ITimer();

			virtual float getDelta()const = 0;
			virtual const float * getDeltaAddress() const = 0;
			virtual float getCurrentDelta() const = 0;
			static ITimer * construct();
		};

	}

}

#endif /* ITIMER_H_ */
