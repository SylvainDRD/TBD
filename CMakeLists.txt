cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
cmake_policy(SET CMP0043 NEW)
project(TBD VERSION 0.0.1 DESCRIPTION "")

add_definitions(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

file(GLOB_RECURSE sources ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(binary TBD)
add_executable(${binary} ${sources})
target_include_directories(${binary} PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DPROJECT_DEBUG)
endif()

target_compile_features(${binary} PRIVATE cxx_std_17) 

# External dependencies
find_package(Vulkan REQUIRED)

set(external ${CMAKE_SOURCE_DIR}/external)

add_subdirectory(${external}/glm)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)
target_include_directories(${binary} PRIVATE ${external}/glfw/include)

add_subdirectory(${external}/vk-bootstrap)
target_link_libraries(${binary} glfw vk-bootstrap::vk-bootstrap)
add_dependencies(${binary} glfw)
