cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
cmake_policy(SET CMP0043 NEW)

set(binary TBD)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
project(${binary} VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} DESCRIPTION "")

add_definitions(-DPROJECT_NAME="${binary}" 
				-DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} 
				-DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
				-DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
				-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

file(GLOB_RECURSE sources 	${CMAKE_SOURCE_DIR}/src/*.cpp
							${CMAKE_SOURCE_DIR}/src/general/*.cpp
							${CMAKE_SOURCE_DIR}/src/utils/*.cpp
							${CMAKE_SOURCE_DIR}/src/renderer/*.cpp
							${CMAKE_SOURCE_DIR}/src/renderer/vulkan/*.cpp
						  )

add_executable(${binary} ${sources})
target_include_directories(${binary} PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(NOT CMAKE_BUILD_TYPE MATCHES ".*Release.*")
	add_definitions(-DPROJECT_DEBUG)
endif()

target_compile_features(${binary} PRIVATE cxx_std_17) 

# External dependencies
find_package(Vulkan REQUIRED)

set(external ${CMAKE_SOURCE_DIR}/external)

add_subdirectory(${external}/glm)
target_include_directories(${binary} PRIVATE ${external}/glm)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)
target_include_directories(${binary} PRIVATE ${external}/glfw/include)

target_link_libraries(${binary} glfw vulkan)
add_dependencies(${binary} glfw)
