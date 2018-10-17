#ifndef CVR_GLESDRAWABLE_H
#define CVR_GLESDRAWABLE_H

#include <osg/Drawable>
#include <osg/Geode>
#include <stack>
#include <cvrUtil/AndroidHelper.h>
namespace cvr{
    static void checkGlError(const char* op) {
        for (GLint error = glGetError(); error; error
                                                        = glGetError()) {
            LOGI("after %s() glError (0x%x)\n", op, error);
        }
    }
    extern GLuint CreateProgram(const char* pVertexSource, const char* pFragmentSource);

    class glesDrawable: public osg::Drawable {
    protected:
        std::stack<cvr::glState>* _stateStack;
        bool PushAllState() const;
        bool PopAllState() const;
        osg::ref_ptr<osg::Geode> glNode;
        GLuint _shader_program;

        void _init(assetLoader* loader,
                            const char* vshader_file, const char* fshader_file){

            std::string vshader, fshader;
            if(loader->getShaderSourceFromFile(vshader_file,fshader_file,vshader,fshader))
                _shader_program = CreateProgram(vshader.c_str(), fshader.c_str());
            else
                LOGE("Fail to load shader or create shader program");
        }
    public:
        virtual void Initialization(assetLoader * loader, std::stack<cvr::glState>*& stateStack){
            _stateStack = stateStack;
        }


        osg::ref_ptr<osg::Geode> createDrawableNode(assetLoader * loader,
                                                    std::stack<cvr::glState>* stateStack){
            Initialization(loader, stateStack);
            glNode = new osg::Geode;
            glNode->addDrawable(this);
            setUseDisplayList(false);
            return glNode.get();
        }
        osg::ref_ptr<osg::Geode> getGLNode(){return glNode;}
    };
}
#endif
