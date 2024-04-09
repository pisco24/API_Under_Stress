# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.2.0)
project(cppREST VERSION 0.1.0)

find_package(Crow)
target_link_libraries(your_project PUBLIC Crow::Crow)

# Set your project name
project(CrowApp)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(crow_app src/main.cpp)

# Link against pthread and boost_system libraries
target_link_libraries(crow_app pthread boost_system)


include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(lib/cppMongo)

include_directories(/usr/local/homebrew/Cellar/boost/1.78.0_1/include)
link_directories(/usr/local/homebrew/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Crow/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cpp_redis/includes)

add_executable(cppREST main.cpp)

target_link_libraries(cppREST boost_system)
target_link_libraries(cppREST pthread)
target_link_libraries(cppREST cpp_redis)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)