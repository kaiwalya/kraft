cmake_minimum_required(VERSION 2.8)

project("kq")

include_directories("../code/include")

add_executable("kq"

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
