file(GLOB GraphEditorSource *.cxx)

include_directories(include)

add_executable(GraphEditor ${GraphEditorSource})
target_link_libraries(GraphEditor ImGui glfw)
