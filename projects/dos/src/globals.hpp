
#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#include "layout.hpp"


namespace dos{
	class Globals{
	public:
		Globals();
		~Globals();
		dos::utils::Logger *l;
		dos::utils::DynamicVariables *vars;
		dos::system::Engine *e;
	};

	enum LogLevels{
		llReport = 2,
		llWarn = 1,
		llError = 0,
	};

	extern Globals * g_pGlobal;
}


#define dos_log(rest...) dos::g_pGlobal->l->log(dos::llReport, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##rest)
#define dos_warn(rest...) dos::g_pGlobal->l->log(dos::llWarn, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##rest)
#define dos_err(rest...) dos::g_pGlobal->l->log(dos::llError, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##rest)
#define dos_logLevel(level, rest...) dos::g_pGlobal->l->log(level, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##rest)


#endif /* GLOBALS_HPP_ */
