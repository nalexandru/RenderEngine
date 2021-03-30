file(GLOB TestProgramSource *.cxx)

add_executable(TestProgram ${TestProgramSource})
target_link_libraries(TestProgram RenderEngine)
