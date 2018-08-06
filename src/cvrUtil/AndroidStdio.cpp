#include <cvrUtil/AndroidStdio.h>
#include <algorithm>
androidbuf::androidbuf(int priority) {
  
  _logType = static_cast<android_LogPriority>(std::clamp(priority, 0, 8));
  memset(buffer, 0, bufsize);
  this->setp(buffer, buffer + bufsize - 1);
}

int androidbuf::overflow(int c) {
  if (c == traits_type::eof()) {
    *this->pptr() = traits_type::to_char_type(c);
    this->sbumpc();
  }
  return this->sync() ? traits_type::eof() : traits_type::not_eof(c);
}

int androidbuf::sync() {
  int rc = 0;
  if (this->pbase() != this->pptr()) {
    __android_log_print(_logType,
                        "Native",
                        "%s",
                        std::string(this->pbase(),
                                    this->pptr() - this->pbase()).c_str());
    rc = 0;
    this->setp(buffer, buffer + bufsize - 1);
  }
  return rc;
}
