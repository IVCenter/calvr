#ifndef BUBBLE_MENU_SUB_MENU_GEOMETRY_H
#define BUBBLE_MENU_SUB_MENU_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

namespace cvr
{
    
class BubbleMenuSubMenuGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuSubMenuGeometry(bool head);
        virtual ~BubbleMenuSubMenuGeometry();

        virtual void selectItem(bool on);
        virtual void openMenu(bool open);
        virtual void createGeometry(MenuItem * item);
        virtual void processEvent(InteractionEvent * event);

        virtual bool isMenuHead();
        virtual bool isMenuOpen();
        virtual void resetMenuLine(float width);

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        bool _head;
        bool _open;

        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeLine;
        osg::ref_ptr<osg::Geode> _geodeIcon;
        osg::ref_ptr<osg::Texture2D> _openIcon;
        osg::ref_ptr<osg::Texture2D> _closedIcon;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
