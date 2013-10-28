#ifndef BUBBLE_MENU_TEXT_BUTTON_SET_GEOMETRY_H
#define BUBBLE_MENU_TEXT_BUTTON_SET_GEOMETRY_H

#include <cvrMenu/BubbleMenu/BubbleMenuGeometry.h>

#include <osgText/Text>
#include <osg/Geode>
#include <osg/MatrixTransform>

#include <map>
#include <string>

namespace cvr
{

class MenuTextButtonSet;

class BubbleMenuTextButtonSetGeometry : public BubbleMenuGeometry
{
    public:
        BubbleMenuTextButtonSetGeometry();
        virtual ~BubbleMenuTextButtonSetGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void processEvent(InteractionEvent * event);

        virtual void updateGeometry();
        virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd);

        virtual void showHoverText();
        virtual void hideHoverText();

    protected:
        struct TextButtonGeometry
        {
                osg::ref_ptr<osg::MatrixTransform> root;
                osg::ref_ptr<osg::MatrixTransform> textTransform;
                osg::ref_ptr<osg::Geode> quad;
                osg::ref_ptr<osg::Geode> quadSelected;
                osg::ref_ptr<osg::Geode> text;
                osg::ref_ptr<osg::Geode> textSelected;
        };

        TextButtonGeometry * createButtonGeometry(std::string text);
        void updateButtons(MenuTextButtonSet * tbs);

        std::map<std::string,TextButtonGeometry*> _buttonMap;

        std::string _intersectedButton;

        float _rowHeight;
        float _buttonWidth;
        float _spacing;

        osg::ref_ptr<osg::Geode> _textGeode;
};

}

#endif
