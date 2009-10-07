
#ifndef OOPS_EXCEPTION_H_
#define OOPS_EXCEPTION_H_
#include "exception"

namespace dos {

	namespace oops {
		enum ExceptionType{
			etGeneric,
			etUserCancel,
			etApiFailure,
			etNotImplemented,
			etMissingCapabilityException,
			etBadProgrammingException,
			etBadParameterException,
			et_MAX,
		};




		class Exception:public std::exception{
		protected:
			const char * m_pMessage;
			enum ExceptionType m_type;
			static const char ** m_sArrExceptionDescriptions;
		public:
			Exception(const char * pMessage = 0);
			virtual const char* what() const throw();
			virtual void diagnose(void **);
		};



		class UserCancelException: public Exception{
		public:
			UserCancelException(const char * pMessage = 0);
		};


		class APIFailureException: public Exception{
		public:
			APIFailureException(const char * pMessage = 0);
		};

		class NotImplementedException: public Exception{
		public:
			NotImplementedException(const char * pMessage = 0);
		};

		class MissingCapabilityException: public Exception{
		public:
			MissingCapabilityException(const char * pMessage = 0);
		};

		class BadProgrammingException: public Exception{
		public:
			BadProgrammingException(const char * pMessage = 0);
		};

		class BadParameterException:public Exception{
		public:
			BadParameterException(const char * pMessage = 0);
		};
	}

}

#endif /* EXCEPTIONA_H_ */
