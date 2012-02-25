import platform;

sSystem = platform.system();

if sSystem == 'Linux':
	sDOSFiles = Split("""
	dos/dos_client.cpp
	dos/globals.cpp
	
	dos/system/Engine.cpp
	dos/system/ITimer.cpp
	dos/system/os/LinuxTimer.cpp
	dos/system/X/XWindows.cpp
	dos/system/IWindowManager.cpp
	
	dos/oops/Exception.cpp
	
	dos/world/TriangleWorld.cpp
	
	dos/utils/logger.cpp
	dos/utils/IVideoGrabber.cpp
	dos/utils/IVideoConverters.cpp
	dos/utils/VideoGrabber.cpp
	dos/utils/VideoGrabberV1.cpp
	dos/utils/DynamicVariables.cpp
	
	dos/utils/comm/IPipe.cpp
	dos/utils/comm/Pipe.cpp
	""");
	envDOS = Environment();
	envDOS.VariantDir('dos', 'projects/dos/src');
	envDOS.Append(CPPPATH = 'dos');
	envDOS.Replace(LIBS = 'GL');
	envDOS.Program('dos_client', sDOSFiles);
	
		
	envWin = Environment();
	envWin.Replace(CC = 'i586-mingw32msvc-gcc')
	envWin.Replace(CXX = 'i586-mingw32msvc-g++')
	
	envKlarity = envWin.Clone();
	envKlarity.VariantDir('klr', 'projects/kq/code/cxx');
	envKlarity.Append(LIBS = 'winmm');
	envKlarity.Program('klarity', ['klr/kqKlarity.cpp', envKlarity.Glob('klr/core_*.cpp')]);
	
	#envCopyP = envWin.Clone();
	#envCopyP.VariantDir('copyp', 'projects/kq/code/source');
	#envCopyP.Program('copyprotect', ['copyp/kqCopyProtect.cpp']);
	

envEXP = Environment();
envEXP.VariantDir('builds/experiments_', 'projects/kq/code/cxx', duplicate=0);
sExpSources = ['builds/experiments_/kqExperiments.cpp', envEXP.Glob('builds/experiments_/core_*.cpp'), 'builds/experiments_/ui_UserInterface.cpp'];
if sSystem == 'Linux':
	sExpSources += ['builds/experiments_/ui_X_UserInterface.cpp'];
	envEXP.Append(LIBS = 'X11:GL');
elif sSystem == 'Darwin':
	envEXP.Append(LIBS = '');
	envEXP.Append(CCFLAGS = '-g -Wall');
	
envEXP.Program('builds/experiments', sExpSources);

sJavaOutDir = 'builds/classes'
envEXP['JARCHDIR']= sJavaOutDir;
envEXP.Java(sJavaOutDir, 'projects/kq/code/java/');

sKQStockClasses = Split("""
	builds/classes/kq/Portfolio$$SymbolInfo.class
	builds/classes/kq/Portfolio.class
	builds/classes/kq/Stock.class
	builds/classes/kq/web/Base64Coder.class
	builds/classes/kq/web/IHTTPResourceCache.class
	builds/classes/kq/web/ZipHttpResourceCache.class
	""");

sKQFlowsDaemonClasses = Split("""
	builds/classes/kq/flows/daemon/Daemon.class
	builds/classes/kq/flows/daemon/ControlPanel.class
	builds/classes/kq/flows/daemon/ControlPanel$$1.class
	builds/classes/kq/flows/daemon/ControlPanel$$2.class
	builds/classes/kq/flows/daemon/ControlPanel$$3.class
	builds/classes/kq/flows/daemon/ControlPanel$$ParametrizedCallable.class
	builds/classes/kq/flows/daemon/ControlPanel$$MainFrame.class
	builds/classes/kq/flows/daemon/Configuration.class
	builds/classes/kq/net/NetworkManager.class
	""");

envEXP.Jar('builds/kq.Stock.jar',sKQStockClasses);
envEXP.Jar('builds/kq.flows.Daemon.jar',sKQFlowsDaemonClasses);


