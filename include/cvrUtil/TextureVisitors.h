#ifndef CALVR_TEXTURE_VISITORS_H
#define CALVR_TEXTURE_VISITORS_H

#include <osg/NodeVisitor>

namespace cvr
{

class TextureResizeNonPowerOfTwoHintVisitor : public osg::NodeVisitor
{
    public:
        TextureResizeNonPowerOfTwoHintVisitor(bool hint);
        ~TextureResizeNonPowerOfTwoHintVisitor();

        virtual void apply(osg::Node& node);
        virtual void apply(osg::Geode& node);

    protected:
        void setHint(osg::StateSet * stateset);
        bool _hint;
};

}

#endif
