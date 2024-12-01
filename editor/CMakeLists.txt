add_executable(
    FizzEditor
    editor.hpp
    editor.cpp
    main.cpp
)

set(ENGINE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/engine/include")

target_include_directories(
    FizzEditor 
    PRIVATE ${ENGINE_INCLUDE_DIR}
    PRIVATE ${SDL2_INCLUDE_DIR})

target_link_libraries(
    FizzEditor 
    PRIVATE FizzEngine
)
