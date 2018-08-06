#ifndef ANDROID_STDIO_H
#define ANDROID_STDIO_H

#ifdef __ANDROID__
#include <android/log.h>
#include <streambuf>

class androidbuf : public std::streambuf {
public:
  enum {
    bufsize = 512
  };
  // 4 for INFO, 6 for ERROR
  androidbuf(int priority);
private:

  android_LogPriority _logType;

  int overflow(int c) override;

  int sync() override;

  char buffer[bufsize];
};

#endif
#endif
