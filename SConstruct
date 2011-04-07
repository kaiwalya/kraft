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


envEXP = Environment();
envDOS.VariantDir('exp', 'projects/kq/code/source');
envEXP.Append(LIBS = 'X11:GL');
envEXP.Program('experiments', ['exp/kqExperiments.cpp', envEXP.Glob('exp/core_*.cpp'), envEXP.Glob('exp/ui_*.cpp')]);


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
