set( project_copyprotect_source
	${kq_source}kqCopyProtect.cpp
)
	
add_executable(${prjCopyProtect} WIN32 ${project_copyprotect_source})
add_dependencies(${prjCopyProtect} ${prjCore})
target_link_libraries(${prjCopyProtect} ${prjCore})

