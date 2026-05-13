cmake_minimum_required(VERSION 3.16)

project(mini_redis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(mini_redis
    src/main.cpp
    src/Database.cpp
)

add_executable(test_database
    tests/test_database.cpp
    src/Database.cpp
)