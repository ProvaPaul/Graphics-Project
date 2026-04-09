cmake_minimum_required(VERSION 3.15)
project(StudentDormitory)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(StudentDormitory
    main.cpp
    globals.cpp
    shaders.cpp
    textures.cpp
    geometry.cpp
    draw_helpers.cpp
    utils.cpp
    input.cpp
    curves.cpp
    scene.cpp
    scene_ground.cpp
    scene_buildings.cpp
    scene_props.cpp
    scene_cricket.cpp
    scene_room.cpp        # ← NEW
)
target_include_directories(StudentDormitory PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(StudentDormitory OpenGL::GL glfw glad)