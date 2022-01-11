cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(WorkStealingThreadPool
        VERSION 1.0.0
        DESCRIPTION "A work stealing thread pool."
        LANGUAGES CXX)

find_package(Threads REQUIRED)
find_package (Boost COMPONENTS unit_test_framework REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                        $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

enable_testing()

add_subdirectory(test)
