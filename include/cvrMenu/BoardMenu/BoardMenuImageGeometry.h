#ifndef BOARD_MENU_IMAGE_GEOMETRY_H
#define BOARD_MENU_IMAGE_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

#include <osg/Geode>
#include <osg/MatrixTransform>

namespace cvr
{

class MenuImage;

class BoardMenuImageGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuImageGeometry();
        virtual ~BoardMenuImageGeometry();

        virtual void selectItem(bool) {}
        virtual void createGeometry(MenuItem * item);
        virtual void processEvent(InteractionEvent * event);
        virtual void updateGeometry();

    protected:
        void updateImage();

        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::MatrixTransform> _mt;

        MenuImage * _mi;
};

}

#endif
