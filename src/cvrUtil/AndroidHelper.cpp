#include <cvrUtil/AndroidHelper.h>
#include <android/log.h>

using namespace cvr;
using namespace osg;

Environment::Environment() = default;

Environment* Environment::_ptr = nullptr;
assetLoader* assetLoader::_myPtr = nullptr;

Environment* Environment::instance() {
  if (!_ptr) {
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
assetLoader*assetLoader::instance() {return _myPtr;}
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

assetLoader::assetLoader(AAssetManager * const assetManager):
_asset_manager(assetManager){
    _myPtr = this;
}

bool assetLoader::LoadTextFileFromAssetManager(const char* file_name,std::string* out_file_text_string) {
    AAsset* asset =
            AAssetManager_open(_asset_manager, file_name, AASSET_MODE_STREAMING);
    if (asset == nullptr) {
        LOGE("Error opening asset %s", file_name);
        return false;
    }

    off_t file_size = AAsset_getLength(asset);
    out_file_text_string->resize(file_size);
    int ret = AAsset_read(asset, &out_file_text_string->front(), file_size);

    if (ret <= 0) {
        LOGE("Failed to open file: %s", file_name);
        AAsset_close(asset);
        return false;
    }

    AAsset_close(asset);
    return true;
}

Program *assetLoader::createShaderProgram(const char *vertShader, const char *fragShader){
    Shader * vs = new Shader(Shader::VERTEX, vertShader);
    Shader * fs = new Shader(Shader::FRAGMENT, fragShader);

    Program *program = new Program;
    program->addShader(vs);
    program->addShader(fs);
    return program;
}

Program* assetLoader::createShaderProgramFromFile(const char* vertex_shader_file_name,
                                          const char* fragment_shader_file_name){
    std::string VertexShaderContent;
    std::string FragmentShaderContent;
    if(getShaderSourceFromFile(vertex_shader_file_name, fragment_shader_file_name, VertexShaderContent, FragmentShaderContent))
        return createShaderProgram(VertexShaderContent.c_str(), FragmentShaderContent.c_str());
    return nullptr;
}

bool assetLoader:: getShaderSourceFromFile(const char* vertex_shader_file_name,
                                                       const char* fragment_shader_file_name,
                                                       std::string & VertexShaderContent,
                                                       std::string & FragmentShaderContent){
    if (!LoadTextFileFromAssetManager(vertex_shader_file_name, &VertexShaderContent)) {
        LOGE("Failed to load file: %s", vertex_shader_file_name);
        return false;
    }

    if (!LoadTextFileFromAssetManager(fragment_shader_file_name, &FragmentShaderContent)) {
        LOGE("Failed to load file: %s", fragment_shader_file_name);
        return false;
    }
    return true;
}

