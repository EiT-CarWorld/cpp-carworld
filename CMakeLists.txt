cmake_minimum_required(VERSION 3.13)  # CMake version check
project(carworld)
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES src/main.cpp src/Window.cpp src/Window.h src/entities/Edge.h src/entities/Node.h src/carMath.cpp src/carMath.h src/rendering/CameraController.cpp src/rendering/CameraController.h src/entities/FloorGrid.cpp src/entities/FloorGrid.h src/rendering/ModelRenderer.cpp src/rendering/ModelRenderer.h src/entities/Node.cpp src/entities/World.cpp src/entities/World.h src/entities/Edge.cpp src/entities/Car.cpp src/entities/Car.h src/rendering/Skybox.cpp src/rendering/Skybox.h src/driving/Route.h src/entities/LineSegment.cpp src/entities/LineSegment.h src/driving/RouteFollower.cpp src/driving/RouteFollower.h src/carConfig.h src/rendering/CarZonesVisualizer.cpp src/rendering/CarZonesVisualizer.h src/UserController.cpp src/UserController.h src/driving/CarBrain.cpp src/driving/CarBrain.h src/Simulation.cpp src/Simulation.h src/GeneticSimulation.cpp src/GeneticSimulation.h src/driving/CarMatrix.h src/driving/CarMatrix.cpp src/util.cpp src/util.h src/entities/AsphaltMesh.cpp src/entities/AsphaltMesh.h)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(carworld ${SOURCE_FILES})
target_include_directories(carworld PRIVATE src/)

# Add raylib as a dependency
add_subdirectory(vendored/raylib)
target_link_libraries(carworld raylib)
target_include_directories(carworld PRIVATE ${raylib_public_headers})
