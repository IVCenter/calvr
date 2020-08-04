#ifndef BUBBLE_MENU_CHECKBOX_GEOMETRY_H
#define BUBBLE_MENU_CHECKBOX_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

namespace cvr
{

class BubbleMenuCheckboxGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuCheckboxGeometry();
        virtual ~BubbleMenuCheckboxGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();
        virtual void processEvent(InteractionEvent * event);

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeIcon;
        osg::ref_ptr<osg::Texture2D> _checkedIcon;
        osg::ref_ptr<osg::Texture2D> _uncheckedIcon;

        osg::ref_ptr<osg::Geode> _checkGeode;
        osg::ref_ptr<osg::Geode> _xGeode;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
