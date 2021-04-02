file(GLOB RenderEngineSource *.cxx)

add_library(RenderEngine SHARED ${RenderEngineSource} "Staging.cxx" "TransientResources.cxx" "vk_mem_alloc.h" "Buffer.cxx")
target_link_libraries(RenderEngine glfw volk)
