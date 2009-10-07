#include "XWindows.hpp"
#include "X11/Xatom.h"
#include "malloc.h"

#include "../../oops/Exception.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"




using namespace dos::system::X;

void XWindows::up(void ** pParams){

	int iWindowHeight = reinterpret_cast<long>(pParams[0]);
	int iWindowWidth = reinterpret_cast<long>(pParams[1]);


	Display * pXDisplay = XOpenDisplay(0);
	if(!pXDisplay){
		throw dos::oops::APIFailureException("XOpenDisplay Fialed");
	}
	Window hWindow = XDefaultRootWindow(pXDisplay);
	if(!hWindow){
		dos::oops::APIFailureException e("XDefaultRootWindow failed");
		throw e;
	}

	GLint attr[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, 0};
	XVisualInfo * pXVisualInfo = glXChooseVisual(pXDisplay, 0, attr);
	if(!pXVisualInfo){
		dos::oops::APIFailureException e("glXChooseVisual failed");
		throw e;
	}

	Colormap cmap = XCreateColormap(pXDisplay, hWindow, pXVisualInfo->visual, AllocNone);
	XSetWindowAttributes xswa = {0};
	xswa.colormap = cmap;
	xswa.event_mask = ExposureMask | KeyPressMask;

	hWindow = XCreateWindow(pXDisplay, hWindow, 0, 0, iWindowWidth, iWindowHeight, 0, pXVisualInfo->depth, InputOutput, pXVisualInfo->visual, CWColormap| CWEventMask, &xswa);
	if(!hWindow){
		throw dos::oops::APIFailureException("XCreateWindow Failed");
	}

	//Register for the window close event
	//so we can exit gracefully
	{
		Atom atoms[1];
		atoms[0] = XInternAtom(pXDisplay, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(pXDisplay, hWindow, atoms, sizeof(atoms)/sizeof(atoms[0]));
		m_atomWindowCloseEvent = atoms[0];
	}





	XMapWindow(pXDisplay, hWindow);

	//Set the window to full screen
	{
		XEvent xev = {0};
		Atom wm_state = XInternAtom(pXDisplay, "_NET_WM_STATE", False);
		Atom fullscreen = XInternAtom(pXDisplay, "_NET_WM_STATE_FULLSCREEN", False);


		xev.type = ClientMessage;
		xev.xclient.window = hWindow;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;
		xev.xclient.data.l[1] = fullscreen;
		xev.xclient.data.l[2] = 0;

		XSendEvent(pXDisplay, DefaultRootWindow(pXDisplay), False, SubstructureNotifyMask, &xev);


	}



	XStoreName(pXDisplay, hWindow, "Denial Of Service");

	GLXContext hContext = glXCreateContext(pXDisplay, pXVisualInfo, 0, GL_TRUE);
	if(!hContext){
		throw dos::oops::APIFailureException("glxCreateContext failed");
	}

	glXMakeCurrent(pXDisplay, hWindow, hContext);

	//Dump OpenGL Environment stuff
	{
		const GLubyte * pVendor = glGetString(GL_VENDOR);
		const GLubyte * pVersion = glGetString(GL_VERSION);
		const GLubyte * pRenderer = glGetString(GL_RENDER);
		const GLubyte * pExtensions = glGetString(GL_EXTENSIONS);

		dos_log("\nOpenGL Environment Information\nVendor:\t %s\nVersion:\t %s\nRenderer:\t %s\nExtension:\t %s", pVendor, pVersion, pRenderer, pExtensions);

	}
	//Initialize the GL Matrices
	{
		float f[4];
		glGetFloatv(GL_VIEWPORT, f);
		float fAspectRatio = (f[2] - f[0])/(f[3]-f[1]);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-fAspectRatio, fAspectRatio, -1, 1, -1, 1);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glPushMatrix();
	}

	//glMatrixMode(GL_TEXTURE);
	//glLoadIdentity();






	m_pXDisplay = pXDisplay;
	m_hWindow = hWindow;
	m_pXVisualInfo = pXVisualInfo;
	m_hContext = hContext;


}

void XWindows::down(void **){


	glXMakeCurrent(m_pXDisplay, 0, 0);

	glXDestroyContext(m_pXDisplay, m_hContext);

	XUnmapWindow(m_pXDisplay, m_hWindow);

	XDestroyWindow(m_pXDisplay, m_hWindow);


	XFree(m_pXVisualInfo->visual->ext_data);
	XFree(m_pXVisualInfo);

	XFlush(m_pXDisplay);

	XCloseDisplay(m_pXDisplay);

}

void XWindows::updateFrame(void **){
	glXSwapBuffers(m_pXDisplay, m_hWindow);
}

void XWindows::process(void ** ){
	int nPendingEvents = XPending(m_pXDisplay);
	XEvent e;
	while(nPendingEvents){

		XNextEvent(m_pXDisplay, &e);

		switch(e.type){
		case Expose:

			break;
		case KeyPress:
			//throw dos::oops::Exception();
			break;
		case ClientMessage:
			if(e.xclient.data.l[0] == (long)m_atomWindowCloseEvent){
				throw dos::oops::UserCancelException("Normal Exit Exception");
			}
			break;
		}
		nPendingEvents--;
	}
}

