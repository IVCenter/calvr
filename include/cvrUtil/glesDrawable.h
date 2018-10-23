#ifndef CVR_GLESDRAWALBE_H
#define CVR_GLESDRAWALBE_H


#include <osg/Drawable>
#include <osg/Geode>
#include <stack>
#include <android/asset_manager.h>
#include <cvrUtil/AndroidHelper.h>
namespace cvr{
    class glesDrawable: public osg::Drawable {
    protected:
        std::stack<cvr::glState>* _stateStack;
        bool PushAllState() const;
        bool PopAllState() const;
        osg::ref_ptr<osg::Geode> glNode;
    public:
        virtual void Initialization(std::stack<cvr::glState>* stateStack){
            _stateStack = stateStack;
        }
        virtual osg::ref_ptr<osg::Geode> createDrawableNode(std::stack<cvr::glState>* stateStack){
            Initialization(stateStack);
            glNode = new osg::Geode;
            glNode->addDrawable(this);
            setUseDisplayList(false);
            return glNode.get();
        }
//        virtual void updateOnFrame();
        osg::ref_ptr<osg::Geode> getGLNode(){return glNode;}
    };
}



#endif

