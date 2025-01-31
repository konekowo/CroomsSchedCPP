cmake_minimum_required(VERSION 3.29)
project(CroomsSchedCPP)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(CroomsSchedCPP WIN32 src/main.cpp)

# Link to the actual SDL3 library.
target_link_libraries(CroomsSchedCPP PRIVATE SDL3::SDL3)
