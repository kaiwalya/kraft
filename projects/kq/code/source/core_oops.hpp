#ifndef KQ_CORE_OOPS_H_

#define KQ_CORE_OOPS_H_


#include "core_oops_OutOfMemoryException.hpp"


#include "core_IntegerTypes.hpp"
namespace kq{
	namespace core{
		namespace oops{
			void assume(kq::core::ui32 iVal, ...);

			class Log{
				bool bLogging;
			public:
				Log();
				virtual void enableLogging();
				virtual void disableLogging();
				void log(const char * format, ...);
			};
		}
	}
}

#endif
