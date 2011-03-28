#include "ui_UserInterface.hpp"
#include "stdio.h"

using namespace kq;
using namespace kq::ui;
using namespace kq::core;
using namespace kq::core::memory;


class UserInterfaceProxy: public UserInterface{
	MemoryWorker mem;
	Pointer<UserInterface> m_pImpl;

protected:
	Pointer< WeakPointer<Screen> > m_arrScreen;
	ui32 m_nScreens;
public:
	UserInterfaceProxy(MemoryWorker memworker):mem(memworker){
		attach(0);
	}

	~UserInterfaceProxy(){
		detach();
	}

	bool attach(Pointer<UserInterface> impl){
		m_pImpl = impl;
		if(m_pImpl){

			m_nScreens = m_pImpl->getScreenCount();
			if(m_nScreens){

				Pointer<WeakPointer<Screen> > pScreens = kq_core_memory_workerRefCountedArrayNew(mem, WeakPointer<Screen>, m_nScreens);
				if(pScreens){
					m_arrScreen = pScreens;
				}
			}
		}
		else{
			m_nScreens = 0;
			m_arrScreen = 0;
		}
		return true;
	}

	Pointer<UserInterface> detach(){
		Pointer<UserInterface> pRet = m_pImpl;
		m_arrScreen = 0;
		m_nScreens = 0;
		m_pImpl = 0;
		return pRet;
	}

	bool refresh(){
		return attach(detach());
	}

	ui32 getScreenCount(){
		return m_nScreens;
	}

	Pointer<ui::Screen> getScreen(ui32 iScreen){
		Pointer<ui::Screen> pRet;
		if(iScreen < m_nScreens)
		{
			pRet = m_arrScreen[iScreen];
			if(!pRet){
				pRet = m_pImpl->getScreen(iScreen);
				m_arrScreen[iScreen] = pRet;
			}
		}
		return pRet;
	}

};

#include "ui_X_UserInterface.hpp"

Pointer<UserInterface> UserInterface::createInstance(MemoryWorker &mem){
    Pointer<UserInterface> pRet;
    pRet = X::createInstance(mem);

    if(pRet){
    	bool bValid = false;
    	Pointer<UserInterfaceProxy> pRet2 = kq_core_memory_workerRefCountedClassNew(mem, UserInterfaceProxy, mem);
    	if(pRet2){
    		if(pRet2->attach(pRet)){
        		pRet = pRet2;
    		}
    	}
    }

    return pRet;
}
