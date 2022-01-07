cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

#https://surfertas.github.io/cmake/cpp/projecteuler/2019/05/01/cmake.html
#https://medium.com/@omaralv/creating-a-shared-library-with-modern-cmake-f332a69bb478
#https://cmake.org/cmake/help/latest/guide/tutorial/Adding%20a%20Library.html

#include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.in" )

#project(WorkStealingThreadPool
#        VERSION ${project_version}
#        DESCRIPTION ${project_description}
#        LANGUAGES CXX)

#set(HEADER_LIST "${WorkStealingThreadPool_SOURCE_DIR}/include/thread_safe_queue.hpp")

#set(SOURCES_LIST "${WorkStealingThreadPool_SOURCE_DIR}/src/thread_safe_queue.cpp")

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#add_library(WorkStealingThreadPool ${HEADER_LIST} ${SOURCES_LIST})

#target_include_directories(${PROJECT_NAME} PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    PRIVATE src)

#target_compile_features(WorkStealingThreadPool PUBLIC cxx_std_20)


#cmake_minimum_required(VERSION 3.5)
#project(mylib VERSION 1.0.0 LANGUAGES CXX)

#set(DEFAULT_BUILD_TYPE "Release")

#if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
#  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
#  # Set the possible values of build type for cmake-gui
#  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
#endif()

#include(GNUInstallDirs)

#set(SOURCE_FILES src/class1.cpp src/class2.cpp)

#add_library(${PROJECT_NAME})

#target_include_directories(${PROJECT_NAME} PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    $<INSTALL_INTERFACE:include>
#    PRIVATE src)

#set_target_properties(${PROJECT_NAME} PROPERTIES
#    VERSION ${PROJECT_VERSION}
#    SOVERSION 1)

#install(TARGETS ${PROJECT_NAME} EXPORT MyLibConfig
#    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
#install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

#install(EXPORT MyLibConfig DESTINATION share/MyLib/cmake)

#export(TARGETS ${PROJECT_NAME} FILE MyLibConfig.cmake)


include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.in" )

project(WorkStealingThreadPool
        VERSION ${project_version}
        DESCRIPTION ${project_description}
        LANGUAGES CXX)


find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

#target_include_directories(
#  ${PROJECT_NAME}
#  INTERFACE $<BUILD_INTERFACE:${${CMAKE_CURRENT_SOURCE_DIR}/include}>
#            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

#target_include_directories(${PROJECT_NAME} INTERFACE include/)

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                        $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

set(Boost_USE_STATIC_LIBS ON)

find_package (Boost COMPONENTS system filesystem unit_test_framework serialization REQUIRED)

enable_testing()

add_subdirectory(test)
