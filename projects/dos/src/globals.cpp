#include "globals.hpp"
#include "utils/logger.hpp"
#include "system/Engine.hpp"
#include "utils/DynamicVariables.hpp"

using namespace dos;

Globals::Globals(){

	l = new dos::utils::Logger();
	l->log(-1, __PRETTY_FUNCTION__, __FILE__, __LINE__, "Initializing Globals");
	e = new dos::system::Engine();
	vars = new dos::utils::DynamicVariables;

}

Globals::~Globals(){
	l->log(-1,__PRETTY_FUNCTION__, __FILE__, __LINE__, "Deallocating Globals");
	delete vars;
	vars = 0;
	delete e;
	e = 0;
	delete l;
	l = 0;

}
