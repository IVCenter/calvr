#ifndef ANDROID_HELPER
#define ANDROID_HELPER

#include <string>
#include <map>

class Environment {
private:
  static Environment* _ptr;
  std::map<std::string, std::string> _env;
  Environment();

public:
  static Environment * instance();
  const char * getVar(const char* name);
  void setVar(std::string key, std::string value);
};

#define getenv(x) __android_getenv(x)
#define setenv(x,y) __android_setenv(x,y)


const char * __android_getenv(const char * name);

void __android_setenv(std::string key, std::string value);
#endif
