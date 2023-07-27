cmake_minimum_required(VERSION 3.10)

project(tinyrhi VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    string(REGEX REPLACE "/W[0-4]\\s*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " /W4 /MP")
endif()

find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found. Please install the Vulkan SDK.")
endif()

set(include_vk
	include/tinyrhi/vulkan.h
	include/tinyrhi/vulkan-device.h
	)
set(src_vk
	src/vulkan/vulkan.cpp
	src/vulkan/vulkan-device.cpp
	)
	
# vulkan
set (tinyrhi_vulkan_target tinyrhi_vk)

add_library(${tinyrhi_vulkan_target} STATIC
	${include_vk}
	${src_vk})

set_target_properties(${tinyrhi_vulkan_target} PROPERTIES FOLDER "TINYRHI")
target_include_directories(${tinyrhi_vulkan_target} PRIVATE include)
target_include_directories(${tinyrhi_vulkan_target} PRIVATE ${Vulkan_INCLUDE_DIR})

if(WIN32)
	target_compile_definitions(${tinyrhi_vulkan_target} PRIVATE 
			VK_USE_PLATFORM_WIN32_KHR
			NOMINMAX)
endif()
target_link_libraries(${tinyrhi_vulkan_target} ${Vulkan_LIBRARY})
	
# install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyrhi
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include)