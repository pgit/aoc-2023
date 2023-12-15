cmake_minimum_required(VERSION 3.17)
project(aoc-2023 VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_CXX_STANDARD 23)

#
# find boost
# https://github.com/madmongo1/blog-december-2020/blob/master/CMakeLists.txt
#
find_package(Boost COMPONENTS system thread atomic coroutine url REQUIRED)
if (Boost_VERSION VERSION_LESS 1.78)
    message(FATAL_ERROR "Boost 1.78 or higher required")
endif ()
find_package(OpenSSL COMPONENTS)
find_package(Threads)

link_libraries(Threads::Threads)
link_libraries(OpenSSL::SSL)
link_libraries(Boost::system Boost::thread Boost::atomic Boost::coroutine Boost::url)

#
# fmt
#
find_package(fmt)
link_libraries(fmt::fmt)

#
# range-v3
#
include_directories(/usr/local/include/range-v3)

#
# targets
#
file(GLOB SOURCE_FILES "src/*.cpp")
foreach(SOURCE_FILE ${SOURCE_FILES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
endforeach()
