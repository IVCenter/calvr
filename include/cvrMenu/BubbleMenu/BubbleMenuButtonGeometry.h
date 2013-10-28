#ifndef BUBBLE_MENU_BUTTON_GEOMETRY_H
#define BUBBLE_MENU_BUTTON_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

namespace cvr
{

class BubbleMenuButtonGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuButtonGeometry();
        virtual ~BubbleMenuButtonGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();
        virtual void processEvent(InteractionEvent * event);

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
