cmake_minimum_required(VERSION 3.14)
project(JamUtil)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

find_package(Vulkan)
find_package(SDL2 REQUIRED)

# All source files are located in the VK2D folder
file(GLOB C_FILES Vulkan2D/VK2D/*.c)
file(GLOB H_FILES Vulkan2D/VK2D/*.h)
set(VMA_FILES Vulkan2D/VulkanMemoryAllocator/src/vk_mem_alloc.h Vulkan2D/VulkanMemoryAllocator/src/VmaUsage.cpp)

include_directories(Vulkan2D/ ${SDL2_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
add_executable(JamUtil main.c JamUtil.c JamUtil.h ${VMA_FILES} ${C_FILES} ${H_FILES})
target_link_libraries(${PROJECT_NAME} m ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})