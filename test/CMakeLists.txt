include_directories (${PROJECT_SOURCE_DIR}}/test
                     ${PROJECT_SOURCE_DIR}/include)

include(${PROJECT_SOURCE_DIR}/cmake/gtest.cmake)

package_add_test(ThreadSafeQueueTest threadsafe_queue.cpp)
package_add_test(WorkStealingThreadPoolTest work_stealing_thread_pool.cpp)