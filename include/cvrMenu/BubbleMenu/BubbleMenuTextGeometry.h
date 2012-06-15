#ifndef BUBBLE_MENU_TEXT_GEOMETRY_H
#define BUBBLE_MENU_TEXT_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

#include <osgText/Text>

namespace cvr
{

class BubbleMenuTextGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuTextGeometry();
        virtual ~BubbleMenuTextGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void processEvent(InteractionEvent * event);
        virtual void updateGeometry();

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osgText::Text> _text;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
