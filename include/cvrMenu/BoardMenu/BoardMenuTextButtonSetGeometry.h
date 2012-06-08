/**
 * @file BoardMenuTextButtonSetGeometry.h
 */
#ifndef BOARD_MENU_TEXT_BUTTON_SET_GEOMETRY_H
#define BOARD_MENU_TEXT_BUTTON_SET_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

#include <osgText/Text>
#include <osg/Geode>
#include <osg/MatrixTransform>

#include <map>
#include <string>

namespace cvr
{

class MenuTextButtonSet;

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu text button set
 */
class BoardMenuTextButtonSetGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuTextButtonSetGeometry();
        virtual ~BoardMenuTextButtonSetGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);

        virtual void processEvent(InteractionEvent * event);

        virtual void updateGeometry();

        virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd);

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
};

/**
 * @}
 */

}

#endif
