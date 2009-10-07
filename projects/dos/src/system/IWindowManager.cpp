#include "IWindowManager.hpp"
#include "X/XWindows.hpp"

using namespace dos::system;

IWindowManager * IWindowManager::construct(){
	return new X::XWindows();
}

IWindowManager::~IWindowManager(){

}
