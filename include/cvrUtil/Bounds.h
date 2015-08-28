#ifndef CALVR_BOUNDS_H
#define CALVR_BOUNDS_H

#include <osg/BoundingBox>
#include <osg/Drawable>
#include <osg/Version>

namespace cvr
{
    static inline osg::BoundingBox getBound(osg::Drawable* drawable)
    {
#if ( OSG_VERSION_LESS_THAN(3, 4, 0) )
        return drawable->getBound();        
#else
        return drawable->getBoundingBox();  
#endif        
    };
}

#endif
