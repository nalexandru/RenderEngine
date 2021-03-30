project(RenderEngine)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate GLFW installation target")

set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "Build PhysFS shared library")
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "Build PhysFS test program")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2 -D_DEBUG -fno-exceptions -fno-rtti -Wall -Wno-int-in-bool-context -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0 -Ofast -fomit-frame-pointer -fno-exceptions -fno-rtti -march=native -mtune=native -Wall -Wno-int-in-bool-context -Werror")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g2 -Ofast -march=native -mtune=native -fno-exceptions -fno-rtti -Wall -Wno-int-in-bool-context -Wall -O0")
 
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g2 -D_DEBUG -Wall -Wno-int-in-bool-context -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0 -Ofast -fomit-frame-pointer -march=native -mtune=native -Wall -Wno-int-in-bool-context -Werror")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g2 -Ofast -march=native -mtune=native -Wall -Wno-int-in-bool-context -Werror -O0")
 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g2")

if(WIN32)
	add_definitions(-DUNICODE)
	set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
else()
	set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()

include_directories($ENV{VK_SDK_PATH}/include)
include_directories(Include)
include_directories(Deps/ImGui)
include_directories(Deps/glfw/include)
include_directories(Deps/volk)

add_subdirectory(Deps/glfw)
add_subdirectory(Deps/ImGui)
add_subdirectory(Deps/volk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Source/RenderEngine)
add_subdirectory(Source/GraphEditor)
add_subdirectory(Source/TestProgram)
