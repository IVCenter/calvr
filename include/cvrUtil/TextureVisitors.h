/**
 * @file TextureVisitors.h
 */

#ifndef CALVR_TEXTURE_VISITORS_H
#define CALVR_TEXTURE_VISITORS_H

#include <osg/NodeVisitor>

namespace cvr
{

/**
 * @addtogroup util
 * @{
 */

/**
 * @brief Node visitor that sets the texture resize hint for all textures
 * in a subgraph
 */
class TextureResizeNonPowerOfTwoHintVisitor : public osg::NodeVisitor
{
    public:
        /**
         * @brief Constructor
         * @param hint value to set for texture resize hint
         */
        TextureResizeNonPowerOfTwoHintVisitor(bool hint);
        ~TextureResizeNonPowerOfTwoHintVisitor();

        virtual void apply(osg::Node& node);
        virtual void apply(osg::Geode& node);

    protected:
        void setHint(osg::StateSet * stateset);
        bool _hint;
};

/**
 * @}
 */

}

#endif
