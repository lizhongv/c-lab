/** @file test_log.c @brief clab_common 的冒烟测试：级别过滤与名称映射。 */

#include <stdio.h>
#include <string.h>

#include "clab/log.h"

int main(void) {
    clab_log_set_level(CLAB_LOG_DEBUG);
    CLAB_DEBUG("debug message: %d", 1);
    CLAB_INFO("info message");
    CLAB_WARN("warn message");
    CLAB_ERROR("error message");

    if (strcmp(clab_log_level_name(CLAB_LOG_DEBUG), "DEBUG") != 0 ||
        strcmp(clab_log_level_name(CLAB_LOG_INFO), "INFO") != 0 ||
        strcmp(clab_log_level_name(CLAB_LOG_WARN), "WARN") != 0 ||
        strcmp(clab_log_level_name(CLAB_LOG_ERROR), "ERROR") != 0) {
        (void)fprintf(stderr, "level name mapping is wrong\n");
        return 1;
    }

    clab_log_set_level(CLAB_LOG_ERROR);
    CLAB_INFO("this line must not appear");
    CLAB_ERROR("after filtering, only ERROR is visible");
    return 0;
}
