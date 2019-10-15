cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/debug")
set(CMAKE_DEBUG_POSTFIX "d")

project (openVulkanoCpp)
add_executable(openVulkanoCpp openVulkanoCpp/main.cpp)

#Setup Vulkan
find_package(Vulkan REQUIRED)
set(Vulkan_LIBRARIES Vulkan::Vulkan)
target_link_libraries(openVulkanoCpp PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(openVulkanoCpp PUBLIC ${Vulkan_INCLUDE_DIR})

if (ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif (WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
else()
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    find_package(XCB REQUIRED)
    link_libraries(${XCB_LIBRARIES})
endif()

set_property(TARGET openVulkanoCpp PROPERTY CXX_STANDARD 14)
target_compile_options(openVulkanoCpp PRIVATE -Wall)

# glfw
if (NOT ANDROID)
add_subdirectory(external/glfw EXCLUDE_FROM_ALL)
add_dependencies(openVulkanoCpp glfw)
target_include_directories(openVulkanoCpp PUBLIC external/glfw/include/GLFW)
target_link_libraries(openVulkanoCpp PRIVATE glfw)
endif()

# glm
include_directories(external/glm)

# fmt
#add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
#target_link_libraries(openVulkanoCpp PRIVATE fmt)

# spdlog
include_directories(external/spdlog/include)
#add_definitions(-SPDLOG_FMT_EXTERNAL)

# assimp
include_directories(external/assimp)
IF(WIN32)
    find_library(ASSIMP_LIBRARIES NAMES assimp libassimp.dll.a PATHS ${CMAKE_SOURCE_DIR}/libs/assimp)
ELSE(WIN32)
    find_package(ASSIMP REQUIRED)
ENDIF(WIN32)

target_sources(openVulkanoCpp PRIVATE openVulkanoCpp/Vulkan/FrameBuffer.cpp openVulkanoCpp/Base/Logger.cpp openVulkanoCpp/Scene/Drawable.cpp openVulkanoCpp/Scene/Node.cpp)

# copy shaders
file(GLOB SHADERS "openVulkanoCpp/Shader/*.spv")
file(COPY ${SHADERS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/Shader/)