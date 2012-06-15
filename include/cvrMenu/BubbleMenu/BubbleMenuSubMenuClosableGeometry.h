#ifndef BUBBLE_MENU_SUB_MENU_CLOSABLE_GEOMETRY_H
#define BUBBLE_MENU_SUB_MENU_CLOSABLE_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuSubMenuGeometry.h>

namespace cvr
{

class BubbleMenuSubMenuClosableGeometry : public BubbleMenuSubMenuGeometry
{
    public:
        BubbleMenuSubMenuClosableGeometry(bool head);
        virtual ~BubbleMenuSubMenuClosableGeometry();

        virtual void createGeometry(MenuItem * item);
        virtual void selectItem(bool on);
        virtual void processEvent(InteractionEvent * event);
        virtual void resetIntersect(float width);
        virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd);

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        bool _overX;

        osg::ref_ptr<osg::Geode> _xGeode;
        osg::ref_ptr<osg::MatrixTransform> _xTransform;
        osg::ref_ptr<osg::Texture2D> _xIcon;
        osg::ref_ptr<osg::Texture2D> _xSelectedIcon;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
