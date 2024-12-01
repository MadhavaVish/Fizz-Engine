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
    
    "${ENGINE_INCLUDE_DIR}/application/window.hpp"
    "${ENGINE_SOURCE_DIR}/application/window.cpp"
)
# set(SDL2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/sdl2/include)
# set(SDL2_LIB_DIR ${CMAKE_SOURCE_DIR}/external/sdl2/lib/x64)

# include_directories(${SDL2_INCLUDE_DIR})
# target_link_directories(
#     FizzEngine
#     PRIVATE ${SDL2_LIB_DIR})

target_include_directories(
    FizzEngine
    PRIVATE ${ENGINE_INCLUDE_DIR}
    PUBLIC ${Vulkan_INCLUDE_DIR}
    # PRIVATE ${SDL2_INCLUDE_DIR}
)
# list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/sdl2/cmake")
# find_package(SDL2 CONFIG REQUIRED)
# find_package(spdlog)

target_link_libraries(
    FizzEngine 
    PUBLIC volk
    PUBLIC SDL2::SDL2 
    PUBLIC spdlog::spdlog
    PRIVATE vk-bootstrap::vk-bootstrap)


target_compile_definitions(FizzEngine PRIVATE FIZZENGINE_EXPORTS)