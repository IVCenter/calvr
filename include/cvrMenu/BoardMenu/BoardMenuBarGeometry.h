#ifndef BOARD_MENU_BAR_GEOMETRY_H
#define BOARD_MENU_BAR_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

#include <osg/MatrixTransform>
#include <osg/Geode>

namespace cvr
{

class MenuBar;

class BoardMenuBarGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuBarGeometry();
        virtual ~BoardMenuBarGeometry();

        virtual void selectItem(bool)
        {
        }
        virtual void processEvent(InteractionEvent * event)
        {
        }
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();
        virtual void resetIntersect(float width);

    protected:
        void updateBar(float width);

        MenuBar * _mb;
        osg::ref_ptr<osg::MatrixTransform> _mt;
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geometry> _geo;
};

}

#endif
