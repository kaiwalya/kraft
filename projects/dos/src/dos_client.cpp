#include "system/Engine.hpp"

#include "utils/logger.hpp"

#include "oops/Exception.hpp"

#include "globals.hpp"

#include "utils/comm/Pipe.hpp"

dos::Globals * dos::g_pGlobal = 0;

#include "malloc.h"
int main(int argc, char ** argv){

	dos::g_pGlobal = new dos::Globals();

	dos::system::Engine *e = (dos::g_pGlobal->e);
	try{
		void * arrParams[] = {&argc, &argv};

		e->up(arrParams);

		try{
			e->process(0);
		}catch(dos::oops::Exception exep){

			e->down(0);
			unsigned int params[] = {dos::llError};
			void * pparams[] = {params};
			exep.diagnose(pparams);
		}

	}catch(dos::oops::Exception exep){
		e->down(0);
		exep.diagnose(0);
		dos_log("Crash during initialization");
	}catch(...){
		e->down(0);
	}

	delete dos::g_pGlobal;
	return 0;
}
