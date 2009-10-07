#include "ITimer.hpp"
#include "os/LinuxTimer.hpp"

using namespace dos::system;

ITimer::~ITimer() {

}

ITimer * ITimer::construct() {
	ITimer * pTimer = 0;
	pTimer = new os::LinuxTimer();
	return pTimer;
	return 0;
}

