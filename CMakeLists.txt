cmake_minimum_required(VERSION 3.16)
project(join_server LANGUAGES CXX VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(Boost 1.70 REQUIRED)

add_executable(join_server src/main.cpp)

target_link_libraries(join_server PRIVATE
    Threads::Threads
    Boost::boost
)

if (MSVC)
    target_compile_options(join_server PRIVATE /W4 /EHsc)
else ()
    target_compile_options(join_server PRIVATE -Wall -Wextra -pedantic)
endif()