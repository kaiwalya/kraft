#include "Engine.hpp"
#include "oops/Exception.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"
#include "utils/DynamicVariables.hpp"
#include "stdio.h"
#include "unistd.h"
#include "world/TriangleWorld.hpp"
#include "string.h"

using namespace dos::system;

Engine::Engine(){
	m_pVideoGrabber = 0;
	m_pCurrentWorld = 0;
	m_pDefaultWorld = 0;
	m_pTimer = 0;
	m_pWindowManager = 0;
}


dos::utils::IVideoGrabber * Engine::getVideoGrabber(){
	return m_pVideoGrabber;
}

void Engine::up(void ** pParams){


	//Load Command line params
	{

		int argc = *reinterpret_cast<int *>(pParams[0]);
		char ** argv = *reinterpret_cast<char ***>(pParams[1]);
		int i = 0;

		while(i < argc){
			bool bValid = true;
			char * pVar, *pVal;
			if(argv[i]){

				int iStrlen = strlen(argv[i]);
				short mark = '-' << 8 | '-';

				if(iStrlen > 2 && (*(short *)argv[i]) == mark){
					pVar = argv[i] + 2;
					pVal = pVar;
					while((*pVal != 0) && (*pVal != '=')){
						pVal++;
					}
					*(pVal) = 0;
					pVal++;

				}else{
					bValid = false;
				}
			}else{
				bValid = false;
			}
			if(bValid){

				dos_log("Adding \"%s\" => \"%s\"", pVar, pVal);
				std::string n = pVar;
				std::string v = pVal;
				dos::g_pGlobal->vars->set(n, dos::utils::DynamicVariables::vtString, &v);
			}else{
				dos_log("Parameter \"%s\" ignored", argv[i]);
			}
			i++;
		}

	}

	//Window Manager
	{

		int iWindowWidth = 300;
		int iWindowHeight = 300;

		//Read configuration

		dos::system::IWindowManager * pWindowManager = 0;
		pWindowManager = pWindowManager->construct();
		void * pUpParams[] = {
				reinterpret_cast<void *>(iWindowWidth),
				reinterpret_cast<void *>(iWindowHeight),
				};

		pWindowManager->up(pUpParams);
		m_pWindowManager = pWindowManager;
	}

	//Timer
	m_pTimer = ITimer::construct();

	//Video Grabber
	try{
		m_pVideoGrabber = dos::utils::IVideoGrabber::construct();
		m_pVideoGrabber->up(0);
	}
	catch(oops::Exception e){
		dos_log("Video Grabber init failed");
		e.diagnose(0);
		m_pVideoGrabber = new dos::utils::NullGrabber();
		m_pVideoGrabber->up(0);
	}

	//Worlds
	{
		m_pDefaultWorld = new dos::world::TriangleWorld;
		m_pCurrentWorld = m_pDefaultWorld;
		m_pDefaultWorld->up(0);
	}
}

void Engine::down(void **){
	if(m_pVideoGrabber){
		m_pVideoGrabber->down(0);
		delete m_pVideoGrabber;
	}
	if(m_pCurrentWorld){
		m_pCurrentWorld->down(0);
		delete m_pCurrentWorld;
	}
	if(m_pDefaultWorld != m_pCurrentWorld){
		m_pDefaultWorld->down(0);
		delete m_pDefaultWorld;
		m_pDefaultWorld = 0;
	}
	m_pCurrentWorld = 0;

	m_pWindowManager->down(0);

	delete m_pWindowManager;
	delete m_pTimer;
	dos_log("Engine Down");
}

void Engine::process(void **){

	int iTargetFrameRatePerSecond = 30;
	float fTargetSecondsPerFrame = 1.0/iTargetFrameRatePerSecond;
	int nFrames = 0;
	int nFramesAtLastShow = 0;
	int iLastSeenVideoFrame = 0;
	float fWorkTime = fTargetSecondsPerFrame;
	float fRunTime = 0;
	float fRunTimeAtLastShow = 0;
	float fShowFrequency = 2.0;
	{

		float fTime;
		void * params[10];

		for(;;){

			//Timing
			m_pTimer->checkpoint();
			fTime = m_pTimer->getDelta();
			fRunTime += fTime;
			params[0] = reinterpret_cast<void *>(&fTime);

			//Process
			{
				//Get the events from the system
				m_pWindowManager->process(params);

				//Video Manager
				m_pVideoGrabber->process(0);

				m_pCurrentWorld->process(0);

			}

			//Move and render the game world
			m_pCurrentWorld->move(0);
			m_pCurrentWorld->render(0);


			//Swap buffers
			m_pWindowManager->updateFrame(0);


			nFrames++;

			float fCurrentWorkTime = m_pTimer->getCurrentDelta();
			//fWorkTime *= 0.2 ;
			fWorkTime += fCurrentWorkTime;

			if(fRunTime - fRunTimeAtLastShow > fShowFrequency){
				fWorkTime/=(nFrames - nFramesAtLastShow);

				utils::VideoGrabber::VideoBuffer b;
				m_pVideoGrabber->getFrame(&b);
				float fVFps = b.iFrameCount - iLastSeenVideoFrame;
				iLastSeenVideoFrame = b.iFrameCount;
				fVFps/=fShowFrequency;

				dos_log("load %.2f%%, %.2fms/%.2fms, fps(possible %.0ffps,maintained %.2f | vid %f)", fWorkTime/fTargetSecondsPerFrame * 100, fWorkTime * 1000, fTargetSecondsPerFrame * 1000, 1/fWorkTime, (1.0f * nFrames - nFramesAtLastShow)/fShowFrequency, fVFps);
				fWorkTime = 0;
				fRunTimeAtLastShow = fRunTime;
				nFramesAtLastShow = nFrames;

			}

			float fSpareTime = fTargetSecondsPerFrame - m_pTimer->getCurrentDelta();

			if(fSpareTime > 0.000001){
				//usleep(fSpareTime * 1000000 * 0.9);
			}

		}
	}

}
