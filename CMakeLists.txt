cmake_minimum_required(VERSION 3.16)
project(template)

# Append the project-specific cmake module path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Detect architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx -Wno-implicit-function-declaration")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
endif()

# Find required packages
find_package(mujoco REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Create the main executable
add_executable(main 
    src/main.c
)

# Link libraries to the main executable
target_link_libraries(main mujoco::mujoco glfw m)

# macOS-specific linking (Apple Silicon fix)
if(APPLE)
    target_link_libraries(main "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
endif()

# Set the output directory inside the build folder
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)