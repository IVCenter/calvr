/**
 * @file BoardMenuSubMenuClosableGeometry.h
 */
#ifndef BOARD_MENU_SUB_MENU_CLOSABLE_GEOMETRY_H
#define BOARD_MENU_SUB_MENU_CLOSABLE_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuSubMenuGeometry.h>

namespace cvr
{

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu closable sub menu
 */
class BoardMenuSubMenuClosableGeometry : public BoardMenuSubMenuGeometry
{
    public:
        BoardMenuSubMenuClosableGeometry(bool head);
        virtual ~BoardMenuSubMenuClosableGeometry();

        virtual void createGeometry(MenuItem * item);

        virtual void selectItem(bool on);
        virtual void processEvent(InteractionEvent * event);
        virtual void resetIntersect(float width);
        virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd);

    protected:
        bool _overX;

        osg::ref_ptr<osg::Geode> _xGeode;
        osg::ref_ptr<osg::MatrixTransform> _xTransform;
        osg::ref_ptr<osg::Texture2D> _xIcon;
        osg::ref_ptr<osg::Texture2D> _xSelectedIcon;
};

/**
 * @}
 */

}

#endif
