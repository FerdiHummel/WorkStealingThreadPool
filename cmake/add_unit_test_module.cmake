macro(add_unit_test_module NAME_OF_MODULE LINK_TARGET)
    set(SOURCE_FILES ${ARGN})
    add_executable (${NAME_OF_MODULE} ${SOURCE_FILES})
    target_link_libraries (${NAME_OF_MODULE}
                           ${LINK_TARGET}
                           ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                           Threads::Threads)
    add_test (NAME ${NAME_OF_MODULE} COMMAND ${NAME_OF_MODULE})
    target_compile_features(${NAME_OF_MODULE} PUBLIC cxx_std_20)
endmacro()

