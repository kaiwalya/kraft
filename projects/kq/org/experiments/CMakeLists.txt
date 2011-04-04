file(GLOB_RECURSE project_experiments_source RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${kq_source}ui*.[c,h]??" "${kq_source}/ui*.[c,h]")

source_group(ui FILES ${project_experiments_source})

set( project_experiments_source
    ${project_experiments_source}
	${kq_source}kqExperiments.cpp

)



add_executable(${prjExperiments} ${project_experiments_source})
add_dependencies(${prjExperiments} ${prjCore})

target_link_libraries(${prjExperiments}
	${prjCore}
	X11
	GL
)

