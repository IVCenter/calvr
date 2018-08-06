#include <cvrUtil/AndroidGetenv.h>

#define let const auto

Environment::Environment() = default;

Environment* Environment::_instance = nullptr;

Environment* Environment::getInstance() {

  if (_instance == nullptr) {
    _instance = new Environment();
#ifdef READ_FROM_FILE
    // TODO


#else
    // hardcode env
    _instance->_env = decltype(_instance->_env){
      {"CALVR_HOST_NAME", "\\"},
      {"CALVR_CONFIG_DIR", "\\"},
      {"CALVR_RESOURCE_DIR", "\\"},
      {"CALVR_PLUGINS_HOME", "\\"},
      {"CALVR_CONFIG_FILE", "\\"},
      {"DISPLAY", "\\"},
      {"CALVR_HOME", "\\"}
      // and the other env.
    };

#endif
  }

  return _instance;
}

const char* Environment::getVar(const char* name ) {
  let it = _instance->_env.find(name);
  if (it == _instance->_env.end()) {
    return NULL;
  }
  return it->second.c_str();
}

const char * __android_getenv(const char * name){
    return Environment::getInstance()->getVar(name);
}
#undef let
