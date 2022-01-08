include_directories (${CMAKE_CURRENT_SOURCE_DIR}}/test
                     ${Boost_INCLUDE_DIRS}
                     ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/add_unit_test_module.cmake)

add_unit_test_module(ThreadSafeQueueTest WorkStealingThreadPool threadsafe_queue.cpp)
add_unit_test_module(WorkStealingThreadPoolTest WorkStealingThreadPool work_stealing_thread_pool.cpp)
