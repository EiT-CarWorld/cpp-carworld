cmake_minimum_required(VERSION 3.13)  # CMake version check
project(carworld)
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES src/main.cpp src/Window.cpp src/Window.h src/entities/Path.h src/entities/Node.h src/carMath.cpp src/carMath.h src/CameraController.cpp src/CameraController.h src/entities/FloorGrid.cpp src/entities/FloorGrid.h src/entities/ModelRenderer.cpp src/entities/ModelRenderer.h src/entities/Node.cpp src/entities/World.cpp src/entities/World.h src/entities/Path.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(carworld ${SOURCE_FILES})
target_include_directories(carworld PRIVATE src/)

# Add raylib as a dependency
add_subdirectory(vendored/raylib)
target_link_libraries(carworld raylib)
target_include_directories(carworld PRIVATE ${raylib_public_headers})
