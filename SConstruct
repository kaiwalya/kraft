import os;

sOS = os.uname()[0];
print 'Operating System is ' + sOS;

sCore = 'kqCore'
sUI = 'kqUI'
sEXP = 'kqExperiments'
sDOS = 'dos_client'


nCore	=	StaticLibrary(sCore,	Glob('projects/kq/code/source/core_*'));
nUI	= 	StaticLibrary(sUI,	Glob('projects/kq/code/source/ui_*'));
nExp	=	Program(sEXP,		['projects/kq/code/source/kqExperiments.cpp', nCore, nUI], LIBS='X11:GL');



dossrc = Split("""
projects/dos/src/dos_client.cpp
projects/dos/src/globals.cpp

projects/dos/src/system/Engine.cpp
projects/dos/src/system/ITimer.cpp
projects/dos/src/system/os/LinuxTimer.cpp
projects/dos/src/system/X/XWindows.cpp
projects/dos/src/system/IWindowManager.cpp

projects/dos/src/oops/Exception.cpp

projects/dos/src/world/TriangleWorld.cpp

projects/dos/src/utils/logger.cpp
projects/dos/src/utils/IVideoGrabber.cpp
projects/dos/src/utils/IVideoConverters.cpp
projects/dos/src/utils/VideoGrabber.cpp
projects/dos/src/utils/VideoGrabberV1.cpp
projects/dos/src/utils/DynamicVariables.cpp

projects/dos/src/utils/comm/IPipe.cpp
projects/dos/src/utils/comm/Pipe.cpp
""")

env = Environment(CPPPATH = 'projects/dos/src', LIBS = 'GL');

nDOS	=	env.Program(sDOS, dossrc);

