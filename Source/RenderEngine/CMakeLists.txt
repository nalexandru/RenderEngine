file(GLOB RenderEngineSource *.cxx)

add_library(RenderEngine SHARED ${RenderEngineSource})
target_link_libraries(RenderEngine glfw volk)
