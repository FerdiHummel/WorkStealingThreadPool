cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(VERSION 1.0.1)

project(WorkStealingThreadPool
        VERSION ${VERSION}
        DESCRIPTION "A work stealing thread pool."
        LANGUAGES CXX)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

include(${PROJECT_SOURCE_DIR}/cmake/install.cmake)

enable_testing()
add_subdirectory(test)