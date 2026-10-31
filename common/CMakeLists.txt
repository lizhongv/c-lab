add_library(clab_common STATIC
    src/log.c
)

target_include_directories(clab_common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

clab_set_warnings(clab_common)
clab_enable_sanitizers(clab_common)

if(CLAB_BUILD_TESTS)
    add_executable(clab_common_test tests/test_log.c)
    target_link_libraries(clab_common_test PRIVATE clab_common)
    clab_set_warnings(clab_common_test)
    clab_enable_sanitizers(clab_common_test)
    add_test(NAME common.log COMMAND clab_common_test)
endif()
