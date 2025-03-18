set(ENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/engine/include")
set(ENGINE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/engine/source")
set(IMGUI_SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
add_library(
    FizzEngine STATIC
    "${ENGINE_INCLUDE_DIR}/engine.hpp"
    "${ENGINE_SOURCE_DIR}/engine.cpp"
    "${ENGINE_INCLUDE_DIR}/foundation/platform.hpp"

    "${ENGINE_INCLUDE_DIR}/foundation/allocators.hpp"
    "${ENGINE_SOURCE_DIR}/foundation/allocators.cpp"

    "${ENGINE_INCLUDE_DIR}/foundation/resource_pool.hpp"
    "${ENGINE_SOURCE_DIR}/foundation/resource_pool.cpp"
    
    "${ENGINE_INCLUDE_DIR}/renderer/renderer.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/renderer.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/vk_types.hpp"

    "${ENGINE_INCLUDE_DIR}/renderer/device.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/device.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/gpu_resources.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/gpu_resources.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/vk_initializers.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/vk_initializers.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/vk_utils.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/vk_utils.cpp"
    
    "${ENGINE_INCLUDE_DIR}/application/window.hpp"
    "${ENGINE_SOURCE_DIR}/application/window.cpp"

    "${IMGUI_SOURCE_DIR}/imgui.h"
    "${IMGUI_SOURCE_DIR}/imgui.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_demo.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_draw.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_tables.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.h"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.h"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp"
)

target_include_directories(
    FizzEngine
    PRIVATE ${ENGINE_INCLUDE_DIR}
    PRIVATE "${CMAKE_SOURCE_DIR}/external/imgui"
    PUBLIC ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(
    FizzEngine 
    PUBLIC volk
    PUBLIC spdlog::spdlog
    PRIVATE SDL2::SDL2 
    PRIVATE vk-bootstrap::vk-bootstrap
)

target_link_libraries(FizzEngine PRIVATE "$ENV{VULKAN_SDK}/Lib/slang.lib")
target_compile_definitions(FizzEngine PRIVATE FIZZENGINE_EXPORTS)