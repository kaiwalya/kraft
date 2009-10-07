cmake_minimum_required(VERSION 2.6)

project("DOS")

set(CMAKE_BUILD_TYPE kdebug)

#set(CMAKE_C_COMPILER llvm-gcc)
#set(CMAKE_CXX_COMPILER llvm-g++)

set(CMAKE_C_FLAGS_KRELEASE "-O3")
set(CMAKE_CXX_FLAGS_KRELEASE "-O3")

set(CMAKE_C_FLAGS_KDEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_KDEBUG "-g -Wall")  

include_directories("../src")

add_executable("dos_client"
../src/dos_client
../src/globals

../src/system/Engine
../src/system/ITimer
../src/system/os/LinuxTimer
../src/system/X/XWindows
../src/system/IWindowManager

../src/oops/Exception

../src/world/TriangleWorld

../src/utils/logger
../src/utils/IVideoGrabber
../src/utils/IVideoConverters
../src/utils/VideoGrabber
../src/utils/VideoGrabberV1
../src/utils/DynamicVariables

../src/utils/comm/IPipe
../src/utils/comm/Pipe

)

target_link_libraries("dos_client"
	"GL"
	"GLU"
)
