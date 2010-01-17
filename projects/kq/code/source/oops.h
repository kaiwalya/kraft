#ifndef _OOPS_H_
#define _OOPS_H_

#include "exception"
namespace oops{
	class OutOfMemoryException: public std::exception{
	};
};


#endif