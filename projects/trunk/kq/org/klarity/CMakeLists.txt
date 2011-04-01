set( project_klarity_source
	${kq_source}kqKlarity.cpp
)
	
add_executable(${prjKlarity} WIN32 ${project_klarity_source})
add_dependencies(${prjKlarity} ${prjCore})
target_link_libraries(${prjKlarity} ${prjCore} winmm)

