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

  void setVar(std::string key, std::string value);
};

#define getenv(x) __android_getenv(x)
#define setenv(x,y) __android_setenv(x,y)


const char * __android_getenv(const char * name);

void __android_setenv(std::string key, std::string value);
#endif
