cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_COMPILE "g++.exe")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fcoroutines")
set(CMAKE_BUILD_TYPE "debug")

project(libco2 VERSION 0.1.0)
add_subdirectory("src/cofunc")
add_subdirectory("src/select")
add_subdirectory("src/schedule")
add_subdirectory("test")
add_executable(main main.cpp)
target_link_libraries(main test schedule select cofunc)

# include(CTest)
# enable_testing()

# add_library(libco2 libco2.cpp)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
