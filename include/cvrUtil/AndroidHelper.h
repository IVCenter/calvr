#ifndef ANDROID_HELPER
#define ANDROID_HELPER

#include <string>
#include <map>
#include <android/asset_manager.h>
#include <osgDB/ReadFile>

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

#define  LOG_TAG    "project"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(tiff)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

namespace cvr{
    class assetLoader{
    private:
        AAssetManager * const _asset_manager;

        bool LoadTextFileFromAssetManager(const char* file_name, std::string* out_file_text_string);

    public:
        assetLoader(AAssetManager * assetManager);

        osg::Program *createShaderProgram(const char *vertShader, const char *fragShader);

        osg::Program* createShaderProgramFromFile(const char* vertex_shader_file_name,
                                     const char* fragment_shader_file_name);
    };
}



#endif
