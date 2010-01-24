
cmake_minimum_required(VERSION 2.8)

project("kq")

add_definitions(-DUNICODE -D_UNICODE)


#Warning level
IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
   # Use the highest warning level for visual studio.
   SET(CMAKE_CXX_WARNING_LEVEL 4)
   IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
     STRING(REGEX REPLACE "/W[0-4]" "/W4"
            CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   ELSE(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
   ENDIF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
ENDIF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")



include_directories("../code/include")

add_executable("kq" WIN32

#core

../code/source/core/IntegerTypes.hpp

../code/source/core/oops.hpp
../code/source/core/oops/OutOfMemoryException.hpp

../code/source/core/Worker.hpp

../code/source/core/memory.hpp
../code/source/core/memory/MemoryWorker.hpp
../code/source/core/memory/PooledMemoryAllocator.cpp
../code/source/core/memory/PooledMemoryAllocator.hpp
../code/source/core/memory/StandardLibraryMemoryAllocator.cpp
../code/source/core/memory/StandardLibraryMemoryAllocator.hpp


../code/source/main
../code/source/main_other


)

target_link_libraries(kq opengl32)
