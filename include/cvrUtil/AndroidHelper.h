#ifndef ANDROID_HELPER
#define ANDROID_HELPER

#include <string>
#include <map>
#include <android/asset_manager.h>
#include <osgDB/ReadFile>
#include <cvrUtil/AndroidStdio.h>
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

////////////////////////////////////////////////////

USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(tiff)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

namespace cvr{
    typedef struct glState_S {
        GLboolean depthTest, blend, cullFace;
        GLboolean dither, colorLogicOp, polygonOffsetLine, polygonOffsetFill;
        GLboolean polygonOffsetPoint, polygonSmooth, scissorTest, stencilTest;
    } glState;

    class assetLoader{
    private:
        AAssetManager * const _asset_manager;

        bool LoadTextFileFromAssetManager(const char* file_name, std::string* out_file_text_string);

    public:
        assetLoader(AAssetManager * assetManager);

        osg::Program *createShaderProgram(const char *vertShader, const char *fragShader);

        osg::Program* createShaderProgramFromFile(const char* vertex_shader_file_name,
                                     const char* fragment_shader_file_name);
        bool getShaderSourceFromFile(const char* vertex_shader_file_name,
                                     const char* fragment_shader_file_name,
                                     std::string & VertexShaderContent,
                                     std::string & FragmentShaderContent);
    };
}



#endif