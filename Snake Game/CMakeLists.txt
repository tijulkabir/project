cmake_minimum_required(VERSION 3.10)
project(GameDevelopment)

# Include header files
include_directories(include)

# Link the library directory
link_directories(lib)

# Add your source files (add glad.c if you're using glad)
add_executable(GameDevelopment
        src/main.cpp
        src/glad.c
)

# Now link the libraries to your target
target_link_libraries(GameDevelopment
        opengl32
        glfw3
)
