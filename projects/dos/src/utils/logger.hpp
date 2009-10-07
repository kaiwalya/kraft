#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_



namespace dos{
	namespace utils{
		class Logger{

			bool (*m_pfnFilter)(unsigned int iLevel);
			static bool defaultFilter(unsigned int iLevel);
		public:
			Logger();
			void log(unsigned int iLevel, const char * pFunc, const char * pSrc, const int iLine, const char * pFormat, ...);
			int setLevelThreshold(unsigned int iLevel);
		};
	}
}

#endif
