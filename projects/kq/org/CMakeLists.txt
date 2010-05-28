
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



file(GLOB_RECURSE groupKQ RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../code/source/*.[c,h]??" "../code/source/*.[c,h]")
source_group("" FILES ${groupKQ})

file(GLOB_RECURSE groupKQCore RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../code/source/core*.[c,h]??" "../code/source/core*.[c,h]")
source_group(core FILES ${groupKQCore})

file(GLOB_RECURSE groupKQFlows RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../code/source/flows*.[c,h]??" "../code/source/flows*.[c,h]")
source_group(flows FILES ${groupKQFlows})

#include_directories("../code/include")

add_executable("kq" ${groupKQ})

target_link_libraries(kq opengl32)
