
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

#Code Generation
IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
ENDIF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")


set(kq_root "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(kq_code "${kq_root}code/")
set(kq_source "${kq_code}source/")
set(kq_include "${kq_code}include/")


include_directories("${kq_include}" "../libs/libxml2/include" "../libs/iconv/include" "../libs/zlib/include")

#file(GLOB_RECURSE project_libKQCore_source RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${kq_source}core*.[c,h]??" "${kq_source}/core*.[c,h]")
#source_group(core FILES ${project_libKQCore_source})


#build Core
set(prjCore "libKQCore")
add_subdirectory("libKQCore")

#build CopyProtect
set(prjCopyProtect "kqCopyProtect")
add_subdirectory("copyprotect")

#build Klarity
set(prjKlarity "kqKlarity")
add_subdirectory("klarity")


#add_executable("kq" "${kq_source}main.cpp")
#add_dependencies("kq" "libKQCore")

#target_link_libraries(kq "libKQCore" opengl32 winmm Ws2_32 ../libs/libxml2/lib/libxml2 ../libs/iconv/lib/iconv ../libs/zlib/lib/zdll)

