include_directories (${CMAKE_CURRENT_SOURCE_DIR}}/test
                     ${Boost_INCLUDE_DIRS}
                     ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable (ThreadSafeQueueTest "threadsafe_queue.cpp" )

target_link_libraries (ThreadSafeQueueTest
                       WorkStealingThreadPool
                       ${Boost_FILESYSTEM_LIBRARY}
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                       ${Boost_SERIALIZATION_LIBRARY}
                       Threads::Threads
                       )

add_test (NAME ThreadSafeQueueTest COMMAND ThreadSafeQueueTest)

target_compile_features(ThreadSafeQueueTest PUBLIC cxx_std_20)

add_executable (WorkStealingThreadPoolTest "work_stealing_thread_pool.cpp" )
target_link_libraries (WorkStealingThreadPoolTest
                       WorkStealingThreadPool
                       ${Boost_FILESYSTEM_LIBRARY}
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                       ${Boost_SERIALIZATION_LIBRARY}
                       Threads::Threads
                       )

target_compile_features(WorkStealingThreadPoolTest PUBLIC cxx_std_20)

add_test (NAME WorkStealingThreadPoolTest COMMAND WorkStealingThreadPoolTest)


#add_custom_target(run_unit_test ALL
#    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
#    DEPENDS ${UNIT_TEST})
