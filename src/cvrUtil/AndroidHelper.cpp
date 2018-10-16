#include <cvrUtil/AndroidHelper.h>

Environment::Environment() = default;

Environment* Environment::_ptr = nullptr;

Environment* Environment::instance() {
  if (_ptr == nullptr) {
      _ptr = new Environment();
      _ptr->_env = decltype(_ptr->_env){
      {"CALVR_HOST_NAME", "\\"},
      {"CALVR_CONFIG_DIR", "\\"},
      {"CALVR_RESOURCE_DIR", "\\"},
      {"CALVR_PLUGINS_HOME", "\\"},
      {"CALVR_CONFIG_FILE", "\\"},
      {"DISPLAY", "\\"},
      {"CALVR_HOME", "\\"}
      // and the other env.
    };
  }
  return _ptr;
}

const char* Environment::getVar(const char* name ) {
  const auto it = _ptr->_env.find(name);
  if (it == _ptr->_env.end()) {
    return nullptr;
  }
  return it->second.c_str();
}

void Environment::setVar(std::string key, std::string value) {
    _ptr->_env[key] = value;
}

const char * __android_getenv(const char * name){
    return Environment::instance()->getVar(name);
}
void __android_setenv(std::string key, std::string value){
    Environment::instance()->setVar(key, value);
}

