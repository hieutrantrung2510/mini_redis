cmake_minimum_required(VERSION 3.16)

project(mini_redis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_executable(test_database
#     tests/test_database.cpp
#     src/Database.cpp
# )

# target_include_directories(test_database PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

add_executable(mini_redis
    src/main.cpp
    src/Database.cpp
    src/TcpServer.cpp
)

target_include_directories(mini_redis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)