#include "Exception.hpp"
#include "../globals.hpp"
#include "utils/logger.hpp"

using namespace dos::oops;

static const char * sArrExceptionDescriptions[et_MAX] = {
			"Generic",
			"User Cancel",
			"API Failure",
			"Not implemented",
			"Missing Capability",
			"Bad Programming",
			"Bad Parameter",
};


const char ** dos::oops::Exception::m_sArrExceptionDescriptions = sArrExceptionDescriptions;

dos::oops::Exception::Exception(const char * pMessage): m_type(etGeneric){
	m_pMessage = pMessage;
	dos_warn("!!!!\t%s", pMessage);
}

const char * dos::oops::Exception::what() const throw(){
	return m_pMessage;
}


void dos::oops::Exception::diagnose(void ** pLevel){
	if(!pLevel){
		dos_log("!!!!\t%s::%s", m_sArrExceptionDescriptions[m_type], m_pMessage);
	}

	if(pLevel){
		unsigned int * params = reinterpret_cast<unsigned int *>(pLevel[0]);
		dos_logLevel(params[0], "!!!!\t%s::%s", m_sArrExceptionDescriptions[m_type], m_pMessage);

	}

}


dos::oops::UserCancelException::UserCancelException(const char * pMessage):Exception(pMessage){
	m_type = etUserCancel;
}

dos::oops::APIFailureException::APIFailureException(const char * pMessage):Exception(pMessage){
	m_type = etApiFailure;
}


dos::oops::NotImplementedException::NotImplementedException(const char * pMessage):Exception(pMessage){
	m_type = etNotImplemented;
}


dos::oops::MissingCapabilityException::MissingCapabilityException(const char * pMessage):Exception(pMessage){
	m_type = etMissingCapabilityException;
}



dos::oops::BadProgrammingException::BadProgrammingException(const char * pMessage):Exception(pMessage){
	m_type = etBadProgrammingException;
}

dos::oops::BadParameterException::BadParameterException(const char * pMessage){
	m_type = etBadParameterException;
}
