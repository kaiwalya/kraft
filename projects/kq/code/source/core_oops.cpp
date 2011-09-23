#include "core_oops.hpp"
#include "stdarg.h"
#include "stdio.h"


using namespace kq::core::oops;

#include "assert.h"

void assume(kq::core::ui32 iVal, ...){
	assert(iVal);
}

class Log{
	bool bLogging;
public:
	Log(){bLogging = false;}
	virtual void enableLogging(){bLogging = true;}
	virtual void disableLogging(){bLogging = false;}
	void log(const char * format, ...){
		if(bLogging){
			va_list args;
			va_start (args, format);
			vprintf (format, args);
			va_end (args);
		}
	}
};
