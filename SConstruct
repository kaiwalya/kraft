import platform;
import os;

sSystem = platform.system();

if sSystem == 'Linux':
	sDOSFiles = Split("""
	builds/dos_client_/dos_client.cpp
	builds/dos_client_/globals.cpp

	builds/dos_client_/system/Engine.cpp
	builds/dos_client_/system/ITimer.cpp
	builds/dos_client_/system/os/LinuxTimer.cpp
	builds/dos_client_/system/X/XWindows.cpp
	builds/dos_client_/system/IWindowManager.cpp

	builds/dos_client_/oops/Exception.cpp

	builds/dos_client_/world/TriangleWorld.cpp

	builds/dos_client_/utils/logger.cpp
	builds/dos_client_/utils/IVideoGrabber.cpp
	builds/dos_client_/utils/IVideoConverters.cpp
	builds/dos_client_/utils/VideoGrabber.cpp
	builds/dos_client_/utils/VideoGrabberV1.cpp
	builds/dos_client_/utils/DynamicVariables.cpp

	builds/dos_client_/utils/comm/IPipe.cpp
	builds/dos_client_/utils/comm/Pipe.cpp
	""");
	envDOS = Environment();
	envDOS.VariantDir('builds/dos_client_', 'projects/dos/src', duplicate=0);
	envDOS.Append(CPPPATH = 'projects/dos/src');
	envDOS.Append(LIBS = 'GL');
	envDOS.Program('builds/dos_client', sDOSFiles);


	envWin = Environment();
	envWin.Replace(CC = 'x86_64-w64-mingw32-gcc')
	envWin.Replace(CXX = 'x86_64-w64-mingw32-g++')
	envWin.Append(LINKFLAGS = '-static-libgcc')
	envWin.Append(LINKFLAGS = '-static-libstdc++')

	envKlarity = envWin.Clone();
	envKlarity.VariantDir('builds/klarity_', 'projects/kq/code/cxx', duplicate=0);
	envKlarity.Append(LIBS = 'winmm');
	envKlarity.Program('builds/klarity', ['builds/klarity_/kqKlarity.cpp', envKlarity.Glob('builds/klarity_/core_*.cpp')]);

	#envCopyP = envWin.Clone();
	#envCopyP.VariantDir('copyp', 'projects/kq/code/source');
	#envCopyP.Program('copyprotect', ['copyp/kqCopyProtect.cpp']);

envEXP = Environment(ENV = os.environ)
#envEXP = Environment();
#print envEXP.environ["PATH"];
envEXP.VariantDir('builds/experiments_', 'projects/kq/code/cxx', duplicate=0);
sExpSources = ['builds/experiments_/kqExperiments.cpp', envEXP.Glob('builds/experiments_/core_*.cpp'), 'builds/experiments_/ui_UserInterface.cpp'];
if sSystem == 'Linux':
	sExpSources += ['builds/experiments_/ui_X_UserInterface.cpp'];
	envEXP.Append(LIBS = 'X11:GL');
elif sSystem == 'Darwin':
	envEXP.Append(LIBS = '');
	envEXP.Append(CCFLAGS = '-g -Wall');
	#envEXP.Append(CXXFLAGS = '-std=c++0x');
	envEXP.Replace(CC = 'cc');
	envEXP.Replace(CXX = 'c++');

envEXP.Program('builds/experiments', sExpSources);

sJavaOutDir = 'builds/classes'
envEXP['JARCHDIR']= sJavaOutDir;
envEXP.Jar('builds/kq.jar', envEXP.Java(sJavaOutDir, 'projects/kq/code/java/'));



