set( project_experiments_source
	${kq_source}kqExperiments.cpp
)
	
add_executable(${prjExperiments} ${project_experiments_source})
add_dependencies(${prjExperiments} ${prjCore})
target_link_libraries(${prjExperiments} ${prjCore})

