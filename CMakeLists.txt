cmake_minimum_required(VERSION 3.15)

# Specifying the build dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

project(DBusTestTask)

add_executable(PermissionManagerWorker src/PermissionManagerWorker.cpp src/services/PermissionManager.cpp
                                       src/common/Utils.cpp)
add_executable(TimeServiceWorker src/TimeServiceWorker.cpp src/services/TimeService.cpp src/common/Utils.cpp)
add_executable(Tester src/Tester.cpp)

# Include headers
target_include_directories(PermissionManagerWorker PUBLIC include)
target_include_directories(TimeServiceWorker PUBLIC include)
target_include_directories(Tester PUBLIC include)

# sdbus-c++ dependency
find_package(sdbus-c++ REQUIRED)
find_package(SQLiteCpp REQUIRED)
target_link_libraries(PermissionManagerWorker PUBLIC SDBusCpp::sdbus-c++)
target_link_libraries(PermissionManagerWorker PUBLIC SQLiteCpp)
target_link_libraries(TimeServiceWorker PUBLIC SDBusCpp::sdbus-c++)
target_link_libraries(Tester PUBLIC SDBusCpp::sdbus-c++)

# Compiler flags and standard setting
add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_17)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(
    compiler_flags INTERFACE "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
                             "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>")
target_link_libraries(PermissionManagerWorker PUBLIC compiler_flags)
target_link_libraries(TimeServiceWorker PUBLIC compiler_flags)
target_link_libraries(Tester PUBLIC compiler_flags)
