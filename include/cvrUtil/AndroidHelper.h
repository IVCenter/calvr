#ifndef ANDROID_HELPER
#define ANDROID_HELPER

#include <string>
#include <map>
#include <android/asset_manager.h>
#include <osgDB/ReadFile>
//#include <cvrUtil/AndroidStdio.h>
#include <cvrUtil/ARCoreManager.h>
#include <stack>
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
USE_OSGPLUGIN(png)
//USE_OSGPLUGIN(obj)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

namespace cvr{
    typedef struct glState_S {
        GLboolean depthTest, blend, cullFace;
        GLboolean dither, colorLogicOp, polygonOffsetLine, polygonOffsetFill;
        GLboolean polygonOffsetPoint, polygonSmooth, scissorTest, stencilTest;
    } glState;
    class glStateStack{
    private:
        static glStateStack * _myPtr;
        std::stack<glState> _glStateStack;
        std::stack<cvr::glState>* _stateStack = &_glStateStack;
    public:
        static glStateStack* instance();
        bool PushAllState() const;
        bool PopAllState() const;
    };

    class envLightCallback:public osg::UniformCallback{
    public:
        virtual void operator()(osg::Uniform* uf, osg::NodeVisitor*nv){
            float *color =  cvr::ARCoreManager::instance()->getLightEstimation().color_correction;
            float intensity = cvr::ARCoreManager::instance()->getLightEstimation().intensity;
            uf->set(osg::Vec4f(color[0], color[1], color[2], intensity));
            uf->dirty();
        }
    };

    class envSHLightCallback:public osg::UniformCallback{
    public:
        virtual void operator()(osg::Uniform* uf, osg::NodeVisitor*nv){
            float *params = ARCoreManager::instance()->getLightEstimation_SH();
            for(int i=0; i<9; i++){
                uf->setElement(i, osg::Vec3f(params[3*i], params[3*i+1], params[3*i+2]));
            }
            uf->dirty();
        }
    };

    class mvpCallback:public osg::UniformCallback{
    public:
        virtual void operator()(osg::Uniform *uf, osg::NodeVisitor *nv){
            uf->set(cvr::ARCoreManager::instance()->getMVPMatrix());
            uf->dirty();
        }
    };
    class modelViewCallBack:public osg::UniformCallback{
        osg::Matrixf _modelMat;
    public:
        modelViewCallBack(osg::Matrixf modelMat):_modelMat(modelMat){}
        virtual void operator()(osg::Uniform *uf, osg::NodeVisitor *nv){
            uf->set(_modelMat * (*cvr::ARCoreManager::instance()->getViewMatrix()));
            uf->dirty();
        }
    };
    class viewMatrixCallback:public osg::UniformCallback{
    public:
        virtual void operator()(osg::Uniform *uf, osg::NodeVisitor *nv){
            uf->set(*cvr::ARCoreManager::instance()->getViewMatrix());
            uf->dirty();
        }
    };

    class projMatrixCallback:public osg::UniformCallback{
    public:
        virtual void operator()(osg::Uniform *uf, osg::NodeVisitor *nv){
            uf->set(*cvr::ARCoreManager::instance()->getProjMatrix());
            uf->dirty();
        }
    };
    class assetLoader{
    private:
        static assetLoader* _myPtr;
        AAssetManager * const _asset_manager;

        bool LoadTextFileFromAssetManager(const char* file_name, std::string* out_file_text_string);
        GLuint _LoadGLShader(GLenum shaderType, const char *pSource);
        GLuint _CreateGLProgramFromSource(const char *pVertexSource, const char *pFragmentSource);
    public:
        static assetLoader * instance();
        assetLoader(AAssetManager * assetManager);

        GLuint createGLShaderProgramFromFile(const char* vert_file, const char *_frag_file);
        osg::Program *createShaderProgram(const char *vertShader, const char *fragShader);

        osg::Program* createShaderProgramFromFile(const char* vertex_shader_file_name,
                                     const char* fragment_shader_file_name);
        bool getShaderSourceFromFile(const char* vertex_shader_file_name,
                                     const char* fragment_shader_file_name,
                                     std::string & VertexShaderContent,
                                     std::string & FragmentShaderContent);

        // Load obj file from assets folder from the app.
        // @param file_name, name of the obj file.
        // @param out_vertices, output vertices.
        // @param out_normals, output normals.
        // @param out_uv, output texture UV coordinates.
        // @param out_indices, output triangle indices.
        // @return true if obj is loaded correctly, otherwise false.
        bool LoadObjFile( const char* file_name,
                          std::vector<GLfloat>* out_vertices,
                          std::vector<GLfloat>* out_normals,
                          std::vector<GLfloat>* out_uv,
                          std::vector<GLushort>* out_indices);
    };
}



#endif
