# Minimum required version of CMake
cmake_minimum_required(VERSION 3.15)

project(FizzEngine VERSION 0.1)
# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
# Find external libs

set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
# Add subdirectories for external libs
add_subdirectory(external/spdlog)
add_subdirectory(external/SDL)
add_subdirectory(external/vk-bootstrap)

if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
add_subdirectory(external/volk)

add_subdirectory(engine)
add_subdirectory(editor)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FizzEditor)
