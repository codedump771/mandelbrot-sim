cmake_minimum_required(VERSION 4.4)
project(mandelbrot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-3.4.8
)

FetchContent_MakeAvailable(SDL3)

add_executable(${PROJECT_NAME} src/mandelbrot.cpp)

set(SDL_INCLUDE_DIR build/_deps/sdl3-src/include)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL_INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -fopenmp)
target_link_options(${PROJECT_NAME} PRIVATE -fopenmp)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)