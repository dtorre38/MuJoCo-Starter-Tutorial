cmake_minimum_required(VERSION 3.16)
project(template)

# Append the project-specific cmake module path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Set the C compiler flags to suppress warnings
set(CMAKE_C_FLAGS "-w -Wno-implicit-function-declaration -mavx")

# Find the Mujoco and GLFW3 packages
find_package(mujoco REQUIRED)
find_package(glfw3 REQUIRED)

# Create the main executable
add_executable(main main.c)

# Link libraries to the main executable
target_link_libraries(main mujoco::mujoco glfw m)
