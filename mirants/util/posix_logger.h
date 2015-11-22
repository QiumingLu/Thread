// Logger implementation that can be shared by all enviroments 
// where enough posix functionality is available.

#ifndef MIRANTS_UTIL_POSIX_LOGGER_H
#define MIRANTS_UTIL_POSIX_LOGGER_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
//#include <algorithm>

#include "include/mirants/env.h"

namespace mirants {

class PosixLogger : public Logger {
 public:
  PosixLogger(FILE* f, uint64_t (*gettid)()) : file_(f), gettid_(gettid) { }
  virtual PosixLogger() {
    fclose(file_);
  }

  virtual void Logv(const char* format, va_list ap);

 private:
  FILE* file_;
  uint64_t (*gettid_)();  // Return the thread id for the current thread.
};

void PosixLogger::Logv(const char* format, va_list ap) {
  const uint64_t thread_id = (*gettid_)();

  // We try twice: the first time with a fixed-size stack allocated buffer,
  // and the second time with a much larger dynamically alocated buffer/
  char buffer[500];
  for (int iter = 0; iter < 2; ++iter) {
    char* base;
    int bufsize;
    if (iter == 0) {
      bufsize = sizeof(buffer);
      base = buffer;
    } else {
      bufsize = 30000;
      base = new char[bufsize];
    }
    char* p = base;
    char* limit = base + bufsize;

    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    const time_t seconds = now_tv.tv_sec;
    struct tm t;
    localtime_r(&seconds, &t);  // 线程安全
    p += snprintf(p, limit - p,
                  "%04d/%02d/%02d-%02d:%02d:%02d.%06d %11x",
                  t.tm_year + 1900,
                  t.tm_mon + 1,
                  t.tm_mday,
                  t.tm_hour,
                  t.tm_min,
                  t.tm_sec,
                  static_cast<int>(now_tv.tv_usec),
                  static_cast<long long unsigned int>(thread_id));

    // Print the message.
    if (p < limit) {
      va_list backup_ap;
      va_copy(backup_ap, ap);
      p += vsnprintf(p, limit - p, format, backup_ap);
      va_end(backup_ap);
    }

    // Truncate to available space if necessary.
    if (p >= limit) {
      if (iter == 0) {
        continue;     // Try again with larger buffer.
      } else {
        p = limit - 1;
      }
    }

    // Add newline if necessary
    if (p == base || p[-1] != '\n') {
      *p++ = '\n';
    }

    assert(p <= limit);
    fwrite(base, 1, p - base, file_);
    fflush(file_);
    if (base != buffer) {
      delete[] base;
    }
    break;
  }
}

}  // namespace mirants

#endif  // MIRANTS_UTIL_POSIX_LOGGER_H
