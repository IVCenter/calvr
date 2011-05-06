#ifndef POPUP_MENU_BASE_H
#define POPUP_MENU_BASE_H

#include <menu/MenuBase.h>

#include <osg/Vec3>
#include <osg/Quat>
#include <osg/Matrix>

namespace cvr
{

class PopupMenuBase : public MenuBase
{
    public:

        virtual void setPosition(osg::Vec3 pos) = 0;
        virtual osg::Vec3 getPosition() = 0;
        virtual void setRotation(osg::Quat rot) = 0;
        virtual osg::Quat getRotation() = 0;
        virtual void setTransform(osg::Matrix m) = 0;
        virtual osg::Matrix getTransform() = 0;

        virtual void setVisible(bool v) = 0;
        virtual bool isVisible() = 0;
};

}
#endif
