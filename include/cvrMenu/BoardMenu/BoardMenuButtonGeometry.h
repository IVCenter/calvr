/**
 * @file BoardMenuButtonGeometry.h
 */
#ifndef BOARD_MENU_BUTTON_GEOMETRY_H
#define BOARD_MENU_BUTTON_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>

namespace cvr
{

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu button
 */
class BoardMenuButtonGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuButtonGeometry();
        virtual ~BoardMenuButtonGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
        virtual void updateGeometry();

        virtual void processEvent(InteractionEvent * event);

    protected:
        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geode> _geodeSelected;
		osg::ref_ptr<osg::Geode> _geodeIcon;
};

/**
 * @}
 */

}

#endif
