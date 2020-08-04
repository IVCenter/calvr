/**
 * @file BoardMenuRadialGeometry.h
 */
#ifndef BOARD_MENU_RADIAL_GEOMETRY_H
#define BOARD_MENU_RADIAL_GEOMETRY_H

#include <cvrMenu/BoardMenu/BoardMenuGeometry.h>
#include <cvrMenu/MenuRadial.h>


namespace cvr
{

/**
 * @addtogroup boardmenu
 * @{
 */

/**
 * @brief Geometry class for a BoardMenu checkbox
 */
class BoardMenuRadialGeometry : public BoardMenuGeometry
{
    public:
        BoardMenuRadialGeometry();
        virtual ~BoardMenuRadialGeometry();

        virtual void selectItem(bool on);
        virtual void createGeometry(MenuItem * item);
		void generateRadial();
        virtual void updateGeometry();
	virtual void update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd);

        virtual void processEvent(InteractionEvent * event);
    protected:
        osg::ref_ptr<osg::Group> _group;
		osg::ref_ptr<osg::MatrixTransform> _selection;

		std::vector<osg::ref_ptr<osg::Geometry> > _innerArcs;
		std::vector<osg::ref_ptr<osg::MatrixTransform> > _outerArcs;
		std::vector<osg::ref_ptr<osgText::Text> > _text;
		std::vector<osg::ref_ptr<osg::Node> > _symbols;
		std::vector<osg::ref_ptr<osg::PositionAttitudeTransform> > _pats;

		std::vector<std::string> _prevText;
		std::vector<bool> _prevIsSymbol;


        float _lastX;
        float _lastY;
	float _center;
	float _radius;
	int _hoverIndex;
	int _selectedIndex;
	MenuRadial * _radial;
};

/**
 * @}
 */

}

#endif
