file(GLOB_RECURSE project_libKQCore_source RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${kq_source}core*.[c,h]??" "${kq_source}/core*.[c,h]")
source_group(core FILES ${project_libKQCore_source})

add_library(${prjCore} ${project_libKQCore_source})


