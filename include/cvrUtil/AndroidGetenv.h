#ifndef ANDROID_GETENV_H
#define ANDROID_GETENV_H

#include <string>
#include <map>


//std::map<std::string, std::string> env_var;

class Environment {
private:

  static Environment* _instance;

  std::map<std::string, std::string> _env;

  Environment();

public:
  static Environment * getInstance();

  const char * getVar(const char* name);

};

#define getenv(x) __android_getenv(x)

const char * __android_getenv(const char * name);

#endif
