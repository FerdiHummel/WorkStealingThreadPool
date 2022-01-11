include(${PROJECT_SOURCE_DIR}/cmake/add_unit_test_module.cmake)

set(Boost_USE_STATIC_LIBS ON)

include_directories (${PROJECT_SOURCE_DIR}}/test
                     ${Boost_INCLUDE_DIRS}
                     ${PROJECT_SOURCE_DIR}/include)

add_unit_test_module(ThreadSafeQueueTest WorkStealingThreadPool threadsafe_queue.cpp)
add_unit_test_module(WorkStealingThreadPoolTest WorkStealingThreadPool work_stealing_thread_pool.cpp)
