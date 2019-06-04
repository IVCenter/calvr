#include <cvrUtil/AndroidHelper.h>
#include <cvrUtil/AndroidStdio.h>

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
    if(getShaderSourceFromFile(vertex_shader_file_name, fragment_shader_file_name, VertexShaderContent, FragmentShaderContent)){
        _CreateGLProgramFromSource(VertexShaderContent.c_str(), FragmentShaderContent.c_str());
        return createShaderProgram(VertexShaderContent.c_str(), FragmentShaderContent.c_str());
    }
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
            LOGE("=====COMPILE SHADER FAILED");
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

bool assetLoader::LoadObjFile(const char *file_name, std::vector<GLfloat> *out_vertices,
                              std::vector<GLfloat> *out_normals, std::vector<GLfloat> *out_uv,
                              std::vector<GLushort> *out_indices) {
    std::vector<GLfloat> temp_positions;
    std::vector<GLfloat> temp_normals;
    std::vector<GLfloat> temp_uvs;
    std::vector<GLushort> vertex_indices;
    std::vector<GLushort> normal_indices;
    std::vector<GLushort> uv_indices;

    std::string file_buffer;
    bool read_success = LoadTextFileFromAssetManager(file_name, &file_buffer);
    if (!read_success) {
        return false;
    }
    std::stringstream file_string_stream(file_buffer);

    while (!file_string_stream.eof()) {
        char line_header[128];
        file_string_stream.getline(line_header, 128);

        if (line_header[0] == 'v' && line_header[1] == 'n') {
            // Parse vertex normal.
            GLfloat normal[3];
            int matches = sscanf(line_header, "vn %f %f %f\n", &normal[0], &normal[1],
                                 &normal[2]);
            if (matches != 3) {
                LOGE("Format of 'vn float float float' required for each normal line");
                return false;
            }

            temp_normals.push_back(normal[0]);
            temp_normals.push_back(normal[1]);
            temp_normals.push_back(normal[2]);
        } else if (line_header[0] == 'v' && line_header[1] == 't') {
            // Parse texture uv.
            GLfloat uv[2];
            int matches = sscanf(line_header, "vt %f %f\n", &uv[0], &uv[1]);
            if (matches != 2) {
                LOGE("Format of 'vt float float' required for each texture uv line");
                return false;
            }

            temp_uvs.push_back(uv[0]);
            temp_uvs.push_back(uv[1]);
        } else if (line_header[0] == 'v') {
            // Parse vertex.
            GLfloat vertex[3];
            int matches = sscanf(line_header, "v %f %f %f\n", &vertex[0], &vertex[1],
                                 &vertex[2]);
            if (matches != 3) {
                LOGE("Format of 'v float float float' required for each vertice line");
                return false;
            }

            temp_positions.push_back(vertex[0]);
            temp_positions.push_back(vertex[1]);
            temp_positions.push_back(vertex[2]);
        } else if (line_header[0] == 'f') {
            // Actual faces information starts from the second character.
            char* face_line = &line_header[1];

            unsigned int vertex_index[4];
            unsigned int normal_index[4];
            unsigned int texture_index[4];

            std::vector<char*> per_vert_info_list;
            char* per_vert_info_list_c_str;
            char* face_line_iter = face_line;
            while ((per_vert_info_list_c_str =
                            strtok_r(face_line_iter, " ", &face_line_iter))) {
                // Divide each faces information into individual positions.
                per_vert_info_list.push_back(per_vert_info_list_c_str);
            }

            bool is_normal_available = false;
            bool is_uv_available = false;
            for (int i = 0; i < per_vert_info_list.size(); ++i) {
                char* per_vert_info;
                int per_vert_infor_count = 0;

                bool is_vertex_normal_only_face =
                        (strstr(per_vert_info_list[i], "//") != nullptr);

                char* per_vert_info_iter = per_vert_info_list[i];
                while ((per_vert_info =
                                strtok_r(per_vert_info_iter, "/", &per_vert_info_iter))) {
                    // write only normal and vert values.
                    switch (per_vert_infor_count) {
                        case 0:
                            // Write to vertex indices.
                            vertex_index[i] = atoi(per_vert_info);  // NOLINT
                            break;
                        case 1:
                            // Write to texture indices.
                            if (is_vertex_normal_only_face) {
                                normal_index[i] = atoi(per_vert_info);  // NOLINT
                                is_normal_available = true;
                            } else {
                                texture_index[i] = atoi(per_vert_info);  // NOLINT
                                is_uv_available = true;
                            }
                            break;
                        case 2:
                            // Write to normal indices.
                            if (!is_vertex_normal_only_face) {
                                normal_index[i] = atoi(per_vert_info);  // NOLINT
                                is_normal_available = true;
                                break;
                            }
                            [[clang::fallthrough]];
                            // Intentionally falling to default error case because vertex
                            // normal face only has two values.
                        default:
                            // Error formatting.
                            LOGE(
                                    "Format of 'f int/int/int int/int/int int/int/int "
                                            "(int/int/int)' "
                                            "or 'f int//int int//int int//int (int//int)' required for "
                                            "each face");
                            return false;
                    }
                    per_vert_infor_count++;
                }
            }

            int vertices_count = per_vert_info_list.size();
            for (int i = 2; i < vertices_count; ++i) {
                vertex_indices.push_back(vertex_index[0] - 1);
                vertex_indices.push_back(vertex_index[i - 1] - 1);
                vertex_indices.push_back(vertex_index[i] - 1);

                if (is_normal_available) {
                    normal_indices.push_back(normal_index[0] - 1);
                    normal_indices.push_back(normal_index[i - 1] - 1);
                    normal_indices.push_back(normal_index[i] - 1);
                }

                if (is_uv_available) {
                    uv_indices.push_back(texture_index[0] - 1);
                    uv_indices.push_back(texture_index[i - 1] - 1);
                    uv_indices.push_back(texture_index[i] - 1);
                }
            }
        }
    }

    bool is_normal_available = (!normal_indices.empty());
    bool is_uv_available = (!uv_indices.empty());

    if (is_normal_available && normal_indices.size() != vertex_indices.size()) {
        LOGE("Obj normal indices does not equal to vertex indices.");
        return false;
    }

    if (is_uv_available && uv_indices.size() != vertex_indices.size()) {
        LOGE("Obj UV indices does not equal to vertex indices.");
        return false;
    }

    for (unsigned int i = 0; i < vertex_indices.size(); i++) {
        unsigned int vertex_index = vertex_indices[i];
        out_vertices->push_back(temp_positions[vertex_index * 3]);
        out_vertices->push_back(temp_positions[vertex_index * 3 + 1]);
        out_vertices->push_back(temp_positions[vertex_index * 3 + 2]);
        out_indices->push_back(i);

        if (is_normal_available) {
            unsigned int normal_index = normal_indices[i];
            out_normals->push_back(temp_normals[normal_index * 3]);
            out_normals->push_back(temp_normals[normal_index * 3 + 1]);
            out_normals->push_back(temp_normals[normal_index * 3 + 2]);
        }

        if (is_uv_available) {
            unsigned int uv_index = uv_indices[i];
            out_uv->push_back(temp_uvs[uv_index * 2]);
            out_uv->push_back(temp_uvs[uv_index * 2 + 1]);
        }
    }

    return true;
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