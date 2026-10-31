/** @file log.c @brief log.h 的实现。 */

/* 严格 C11（-std=c11）不声明 POSIX 的 localtime_r，需要显式请求 POSIX.1-2008。 */
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "clab/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static SRWLOCK g_lock = SRWLOCK_INIT;
#define CLAB_LOCK() AcquireSRWLockExclusive(&g_lock)
#define CLAB_UNLOCK() ReleaseSRWLockExclusive(&g_lock)
#else
#include <pthread.h>
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
#define CLAB_LOCK() (void)pthread_mutex_lock(&g_lock)
#define CLAB_UNLOCK() (void)pthread_mutex_unlock(&g_lock)
#endif

/* 级别阈值在启动阶段设置；运行期修改不属于线程安全约定。 */
static clab_log_level g_level = CLAB_LOG_INFO;
static const char *const LEVEL_NAMES[] = {"DEBUG", "INFO", "WARN", "ERROR"};

void clab_log_set_level(clab_log_level level) { g_level = level; }

const char *clab_log_level_name(clab_log_level level) {
    if (level < CLAB_LOG_DEBUG || level > CLAB_LOG_ERROR) {
        return "UNKNOWN";
    }
    return LEVEL_NAMES[level];
}

static void timestamp(char *out, size_t size) {
    time_t now = time(NULL);
    struct tm local;
#ifdef _WIN32
    (void)localtime_s(&local, &now);
#else
    (void)localtime_r(&now, &local);
#endif
    if (strftime(out, size, "%Y-%m-%d %H:%M:%S", &local) == 0) {
        out[0] = '\0';
    }
}

void clab_log(clab_log_level level, const char *fmt, ...) {
    if (level < g_level || fmt == NULL) {
        return;
    }

    char message[1024];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(message, sizeof message, fmt, args);
    va_end(args);
    if (written < 0) {
        return;
    }

    char now[32];
    timestamp(now, sizeof now);

    CLAB_LOCK();
    (void)fprintf(stderr, "%s %-5s %s\n", now, clab_log_level_name(level), message);
    (void)fflush(stderr);
    CLAB_UNLOCK();
}
