#ifndef _DOS_UTILS_IVIDEOGRABBER_H_
#define _DOS_UTILS_IVIDEOGRABBER_H_

namespace dos{
	namespace utils{
		class IVideoGrabber{


		public:

			enum VideoFormat{
				vfUnknown,
				vf_RGBA_32,
				vf_YUVU_32,
				vf_RGB_24,
				vf_MAX,
			};

			struct VideoBuffer{
				void * address;
				int width;
				int height;
				int length;
				int iFrameCount;
				VideoFormat format;
			};

			static void convertFormat(VideoBuffer * pOut, VideoBuffer * pIn);



			static IVideoGrabber * construct();
			static IVideoGrabber * constructByTesting();
			virtual void up(void **) = 0;
			virtual void process(void **) = 0;
			virtual void down(void **) = 0;
			virtual void getFrame(VideoBuffer *) = 0;
			virtual ~IVideoGrabber(){}


		protected:
			//Video Format convertor function map
			static void (*s_convertfuncs[vf_MAX][vf_MAX])(VideoBuffer * pOut, VideoBuffer * pIn);
		};

		class NullGrabber:public IVideoGrabber{
			virtual void up(void **);
			virtual void process(void **);
			virtual void down(void **);
			virtual void getFrame(VideoBuffer *);

		};
	}
}
#endif
