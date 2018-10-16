#ifndef CVR_GLESDRAWABLE_H
#define CVR_GLESDRAWABLE_H

#include <osg/Drawable>
#include <osg/Geode>
#include <stack>

namespace cvr{
    typedef struct glState_S {
        GLboolean depthTest, blend, cullFace;
        GLboolean dither, colorLogicOp, polygonOffsetLine, polygonOffsetFill;
        GLboolean polygonOffsetPoint, polygonSmooth, scissorTest, stencilTest;
    } glState;

    class glesDrawable: public osg::Drawable {
    protected:
        std::stack<glState>* _stateStack;
        bool PushAllState() const;
        bool PopAllState() const;
        osg::ref_ptr<osg::Geode> glNode;
    public:
        virtual void Initialization(std::stack<glState>* stateStack){
            _stateStack = stateStack;
        }
        osg::ref_ptr<osg::Geode> createDrawableNode(std::stack<glState>* stateStack){
            Initialization(stateStack);
            glNode = new osg::Geode;
            glNode->addDrawable(this);
            setUseDisplayList(false);
            return glNode.get();
        }
        osg::ref_ptr<osg::Geode> getGLNode(){return glNode;}
    };
}
#endif
