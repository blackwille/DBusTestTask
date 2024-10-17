cmake_minimum_required(VERSION 3.15)

# Specifying the build dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

project(DBusTestTask)

add_executable(PermissionManager src/PermissionManager.cpp)

# Include headers
target_include_directories(PermissionManager PUBLIC include)

# sdbus-c++ dependency
find_package(sdbus-c++ REQUIRED)
find_package(SQLiteCpp REQUIRED)
target_link_libraries(PermissionManager PUBLIC SDBusCpp::sdbus-c++)
target_link_libraries(PermissionManager PUBLIC SQLiteCpp)

# Compiler flags and standard setting
add_library(tutorial_compiler_flags INTERFACE)
target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_17)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(
  tutorial_compiler_flags
  INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>")
target_link_libraries(PermissionManager PUBLIC tutorial_compiler_flags)
