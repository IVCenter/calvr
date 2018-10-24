#include <cvrUtil/AndroidHelper.h>
#include <android/log.h>

using namespace cvr;
using namespace osg;

Environment::Environment() = default;

Environment* Environment::_ptr = nullptr;
assetLoader* assetLoader::_myPtr = nullptr;
glStateStack* glStateStack::_myPtr = nullptr;

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
glStateStack*glStateStack::instance() {
    if(!_myPtr) _myPtr=new glStateStack;
    return _myPtr;
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

GLuint assetLoader::_LoadGLShader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char *buf = (char *) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint assetLoader::_CreateGLProgramFromSource(const char *pVertexSource, const char *pFragmentSource) {
    GLuint vertexShader = _LoadGLShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = _LoadGLShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);

        glAttachShader(program, pixelShader);

        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char *buf = (char *) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}
GLuint assetLoader::createGLShaderProgramFromFile(const char* vert_file, const char *_frag_file){
    std::string VertexShaderContent;
    if (!LoadTextFileFromAssetManager(vert_file, &VertexShaderContent)) {
        LOGE("Failed to load file: %s", vert_file);
        return 0;
    }

    std::string FragmentShaderContent;
    if (!LoadTextFileFromAssetManager(_frag_file, &FragmentShaderContent)) {
        LOGE("Failed to load file: %s", _frag_file);
        return 0;
    }

    return _CreateGLProgramFromSource(VertexShaderContent.c_str(), FragmentShaderContent.c_str());
}
bool glStateStack::PushAllState() const
{
    cvr::glState state;

    state.blend = glIsEnabled(GL_BLEND);
    state.depthTest = glIsEnabled(GL_DEPTH_TEST);
    state.cullFace = glIsEnabled(GL_CULL_FACE);
    state.dither = glIsEnabled(GL_DITHER);
    state.polygonOffsetFill = glIsEnabled(GL_POLYGON_OFFSET_FILL);
    state.scissorTest = glIsEnabled(GL_SCISSOR_TEST);
    state.stencilTest = glIsEnabled(GL_STENCIL_TEST);

    _stateStack->push(state);
    return true;
}
bool glStateStack::PopAllState() const
{
    if (!_stateStack->empty()) {
        cvr::glState state = _stateStack->top();

        if (state.blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (state.depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (state.cullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (state.dither) glEnable(GL_DITHER); else glDisable(GL_DITHER);
        if (state.polygonOffsetFill) glEnable(GL_POLYGON_OFFSET_FILL); else glDisable(GL_POLYGON_OFFSET_FILL);
        if (state.scissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (state.stencilTest) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);

        _stateStack->pop();
    }
    return true;
}