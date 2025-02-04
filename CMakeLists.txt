include(FetchContent)
cmake_minimum_required(VERSION 3.29)
project(CroomsSchedCPP)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 23)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(vendored/cpr EXCLUDE_FROM_ALL)


add_executable(CroomsSchedCPP WIN32 src/main.cpp
        src/Text_util.cpp
        src/Text_util.h
        src/Sched_util.cpp
        src/Sched_util.h
)


add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${ASSETS_DIR}"
        "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets"
        COMMENT "Copying assets to build directory"
)

add_dependencies(CroomsSchedCPP copy_assets)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(CroomsSchedCPP PRIVATE SDL3::SDL3)
target_link_libraries(CroomsSchedCPP PRIVATE SDL3_ttf::SDL3_ttf)
target_link_libraries(CroomsSchedCPP PRIVATE cpr::cpr)
target_link_libraries(CroomsSchedCPP PRIVATE nlohmann_json::nlohmann_json)