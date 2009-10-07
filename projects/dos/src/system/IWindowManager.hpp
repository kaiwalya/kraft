#ifndef SYSTEM_IWINDOWMANAGER_H_
#define SYSTEM_IWINDOWMANAGER_H_

namespace dos{
	namespace system{
		class IWindowManager{
		public:
			long getApplicationWindow() const;
			long getApplication() const;
		public:
			static IWindowManager * construct();
		public:
			virtual void up(void **) = 0;
			virtual void down(void **) = 0;
			virtual void process(void **) = 0;
			virtual void updateFrame(void **) = 0;
			virtual ~IWindowManager();
		};
	}
}

#endif
