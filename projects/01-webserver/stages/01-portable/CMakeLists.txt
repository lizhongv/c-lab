add_executable(webserver01
    src/main.c
    src/handle.c
    src/method.c
    src/send.c
    src/file.c
    src/other.c
)

if(WIN32)
    target_sources(webserver01 PRIVATE src/platform_win32.c)
    target_link_libraries(webserver01 PRIVATE ws2_32)
else()
    target_sources(webserver01 PRIVATE src/platform_posix.c)
endif()

target_include_directories(webserver01 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(webserver01 PRIVATE clab_common)

clab_set_warnings(webserver01)
clab_enable_sanitizers(webserver01)

if(CLAB_BUILD_TESTS)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(
            NAME webserver01.integration
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_stage01.py"
                    "$<TARGET_FILE:webserver01>"
        )
        set_tests_properties(webserver01.integration PROPERTIES TIMEOUT 60)
    endif()
endif()
