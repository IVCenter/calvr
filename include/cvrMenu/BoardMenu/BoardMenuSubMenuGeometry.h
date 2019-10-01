/**
 * @file BoardMenuSubMenuGeometry.h
 */
#ifndef BOARD_MENU_SUB_MENU_GEOMETRY_H
#define BOARD_MENU_SUB_MENU_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu sub menu
 */
class BoardMenuSubMenuGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuSubMenuGeometry(bool head);
        virtual ~BoardMenuSubMenuGeometry();

        virtual void selectItem(bool on);
        virtual void openMenu(bool open);
        virtual void createGeometry(MenuItem * item);
		virtual void updateGeometry();

        virtual void processEvent(InteractionEvent * event);

        virtual bool isMenuHead();
        virtual bool isMenuOpen();

        virtual void resetMenuLine(float width);

    protected:
        bool _head;
        bool _open;

        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
        osg::ref_ptr<osg::Geode> _geodeLine;
        osg::ref_ptr<osg::Geode> _geodeIcon;
        osg::ref_ptr<osg::Texture2D> _openIcon;
        osg::ref_ptr<osg::Texture2D> _closedIcon;
};

/**
 * @}
 */

}

#endif
