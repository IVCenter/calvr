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

        virtual void Initialization();
        void createShaderProgram(const char* vshader_file, const char* fshader_file);
    public:
        glesDrawable(std::stack<cvr::glState>* stateStack);
        ~glesDrawable();

        virtual osg::ref_ptr<osg::Geode> createDrawableNode();

        osg::ref_ptr<osg::Geode> getGLNode(){return glNode;}
    };
}
#endif
