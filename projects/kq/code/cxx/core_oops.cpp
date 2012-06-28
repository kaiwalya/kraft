#include "core_oops.hpp"
#include "stdarg.h"
#include "stdio.h"


using namespace kq::core::oops;

#include "assert.h"

void kq::core::oops::assume(bool val, ...){
	assert(val);
}

Log::Log(){bLogging = true;}
void Log::enableLogging(){bLogging = true;}
void Log::disableLogging(){bLogging = false;}
void Log::log(const char * format, ...){
	if(bLogging){
		va_list args;
		va_start (args, format);
		vprintf (format, args);
		va_end (args);
	}
}
