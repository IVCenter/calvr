#ifndef BUBBLE_MENU_IMAGE_GEOMETRY_H
#define BUBBLE_MENU_IMAGE_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

#include <osg/Geode>
#include <osg/MatrixTransform>

namespace cvr
{

class MenuImage;

class BubbleMenuImageGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuImageGeometry();
        virtual ~BubbleMenuImageGeometry();

        virtual void selectItem(bool)
        {
        }
        virtual void createGeometry(MenuItem * item);
        virtual void processEvent(InteractionEvent * event);
        virtual void updateGeometry();

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        void updateImage();

        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::MatrixTransform> _mt;

        MenuImage * _mi;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
