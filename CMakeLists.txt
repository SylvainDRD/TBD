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
				-DPROJECT_DIR="${CMAKE_SOURCE_DIR}/")

file(GLOB_RECURSE sources 	${CMAKE_SOURCE_DIR}/src/*.cpp
							${CMAKE_SOURCE_DIR}/src/general/*.cpp
							${CMAKE_SOURCE_DIR}/src/utils/*.cpp
							${CMAKE_SOURCE_DIR}/src/renderer/*.cpp
							${CMAKE_SOURCE_DIR}/src/renderer/rendering_dag/*.cpp
							${CMAKE_SOURCE_DIR}/src/renderer/vulkan/*.cpp
						  )

add_executable(${binary} ${sources})
target_include_directories(${binary} PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(NOT CMAKE_BUILD_TYPE MATCHES ".*Release.*")
	add_definitions(-DPROJECT_DEBUG)
endif()

target_compile_features(${binary} PRIVATE cxx_std_20) 

# External dependencies
find_package(Vulkan REQUIRED)

target_include_directories(${binary} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${binary} ${Vulkan_LIBRARIES})

set(external ${CMAKE_SOURCE_DIR}/external)

add_subdirectory(${external}/glm)
target_include_directories(${binary} PRIVATE ${external}/glm)

add_subdirectory(${external}/glfw)
target_include_directories(${binary} PRIVATE ${external}/glfw/include)
target_link_libraries(${binary} glfw)
add_dependencies(${binary} glfw)

add_subdirectory(${external}/VulkanMemoryAllocator)
target_include_directories(${binary} PRIVATE ${external}/VulkanMemoryAllocator/include)
target_link_libraries(${binary} GPUOpen::VulkanMemoryAllocator)
