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
	envKlarity.VariantDir('klr', 'projects/kq/code/source');
	envKlarity.Append(LIBS = 'winmm');
	envKlarity.Program('klarity', ['klr/kqKlarity.cpp', envKlarity.Glob('klr/core_*.cpp')]);
	
	#envCopyP = envWin.Clone();
	#envCopyP.VariantDir('copyp', 'projects/kq/code/source');
	#envCopyP.Program('copyprotect', ['copyp/kqCopyProtect.cpp']);
	

envEXP = Environment();
envEXP.VariantDir('builds/exp', 'projects/kq/code/source', duplicate=0);
sExpSources = ['builds/exp/kqExperiments.cpp', envEXP.Glob('builds/exp/core_*.cpp'), 'builds/exp/ui_UserInterface.cpp'];
if sSystem == 'Linux':
	sExpSources += ['exp/ui_X_UserInterface.cpp'];
	envEXP.Append(LIBS = 'X11:GL');
elif sSystem == 'Darwin':
	envEXP.Append(LIBS = '');
	envEXP.Append(CCFLAGS = '-g -Wall');
	
envEXP.Program('experiments', sExpSources);

