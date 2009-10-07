
#include "LinuxTimer.hpp"
#include "../../oops/Exception.hpp"


using namespace dos::system::os;


LinuxTimer::LinuxTimer(){
	m_tv.tv_sec = 0;
	m_tv.tv_usec = 0;
	gettimeofday(&m_tv, 0);
	fDelta = 0;
}

float LinuxTimer::_getDelta(timeval * pTV) const{
	return (pTV->tv_sec - m_tv.tv_sec) + (pTV->tv_usec - m_tv.tv_usec)*0.000001f;
}

void LinuxTimer::_capture(timeval * pTV) const{
	gettimeofday(pTV, 0);
}

void LinuxTimer::_update(timeval * pTV){
	m_tv.tv_sec = pTV->tv_sec;
	m_tv.tv_usec = pTV->tv_usec;

}

void LinuxTimer::checkpoint(){
	timeval tv;
	_capture(&tv);
	fDelta = _getDelta(&tv);
	_update(&tv);


}

float LinuxTimer::getDelta() const{
	return fDelta;
}

const float * LinuxTimer::getDeltaAddress() const{
	return &fDelta;
}

float LinuxTimer::getCurrentDelta() const{
	timeval tv;
	_capture(&tv);
	return _getDelta(&tv);
}
