set(ENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/engine/include")
set(ENGINE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/engine/source")

add_library(
    FizzEngine SHARED
    "${ENGINE_INCLUDE_DIR}/engine.hpp"
    "${ENGINE_SOURCE_DIR}/engine.cpp"
    "${ENGINE_INCLUDE_DIR}/foundation/platform.hpp"
    
    "${ENGINE_INCLUDE_DIR}/renderer/renderer.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/renderer.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/vk_types.hpp"

    "${ENGINE_INCLUDE_DIR}/renderer/device.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/device.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/vk_initializers.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/vk_initializers.cpp"

    "${ENGINE_INCLUDE_DIR}/renderer/vk_utils.hpp"
    "${ENGINE_SOURCE_DIR}/renderer/vk_utils.cpp"
    
    "${ENGINE_INCLUDE_DIR}/application/window.hpp"
    "${ENGINE_SOURCE_DIR}/application/window.cpp"
)

target_include_directories(
    FizzEngine
    PRIVATE ${ENGINE_INCLUDE_DIR}
    PUBLIC ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(
    FizzEngine 
    PUBLIC volk
    PUBLIC SDL2::SDL2 
    PUBLIC spdlog::spdlog
    PRIVATE vk-bootstrap::vk-bootstrap
)

target_compile_definitions(FizzEngine PRIVATE FIZZENGINE_EXPORTS)