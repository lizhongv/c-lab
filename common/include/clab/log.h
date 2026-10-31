/**
 * @file log.h
 * @brief 跨平台日志：分级、带时间戳、默认输出到 stderr。
 *
 * 约定：
 *  - 线程安全：内部对一行日志加锁，可多线程调用。
 *  - 不分配内存、不写文件；日志文件与轮转由调用方决定。
 *  - 级别低于当前阈值的调用会被直接丢弃，代价接近一次比较。
 */

#ifndef CLAB_LOG_H
#define CLAB_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum clab_log_level {
    CLAB_LOG_DEBUG = 0,
    CLAB_LOG_INFO = 1,
    CLAB_LOG_WARN = 2,
    CLAB_LOG_ERROR = 3
} clab_log_level;

/** 设置全局最低输出级别，默认 CLAB_LOG_INFO。 */
void clab_log_set_level(clab_log_level level);

/** 输出一行日志；fmt 为 printf 风格格式串。 */
void clab_log(clab_log_level level, const char *fmt, ...);

#define CLAB_DEBUG(...) clab_log(CLAB_LOG_DEBUG, __VA_ARGS__)
#define CLAB_INFO(...) clab_log(CLAB_LOG_INFO, __VA_ARGS__)
#define CLAB_WARN(...) clab_log(CLAB_LOG_WARN, __VA_ARGS__)
#define CLAB_ERROR(...) clab_log(CLAB_LOG_ERROR, __VA_ARGS__)

/** 级别的短名称：DEBUG / INFO / WARN / ERROR。 */
const char *clab_log_level_name(clab_log_level level);

#ifdef __cplusplus
}
#endif

#endif /* CLAB_LOG_H */
