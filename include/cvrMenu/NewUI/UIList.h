#ifndef CALVR_UI_LIST_H
#define CALVR_UI_LIST_H

#include <cvrMenu/Export.h>
#include <cvrMenu/NewUI/UIElement.h>

#include <limits>

namespace cvr
{
	class CVRMENU_EXPORT UIList : public UIElement
	{
	public:
		enum Direction {
			LEFT_TO_RIGHT,
			TOP_TO_BOTTOM
		};
		enum OverflowBehavior {
			CUT,
			CONTINUE,
			WRAP,
			WRAP_REVERSE
		};


		UIList(Direction d = LEFT_TO_RIGHT, OverflowBehavior o = CUT)
			: UIElement()
		{
			setDirection(d);
			setOverflow(o);
			_childBoxes = std::map<UIElement*, std::pair<osg::Vec3, osg::Vec3> >();
			_minSize = 0;
			_maxSize = std::numeric_limits<float>::max();
		}


		virtual void updateElement(osg::Vec3 pos, osg::Vec3 size) override;
		virtual void addChild(UIElement* e) override;
		virtual void removeChild(UIElement* e) override;

		virtual void setDirection(Direction direction);
		virtual Direction getDirection();

		virtual void setOverflow(OverflowBehavior behavior);
		virtual OverflowBehavior getOverflow();

		virtual void setMinSize(float size);
		virtual void setMaxSize(float size);

	protected:
		//osg::ref_ptr<osg::MatrixTransform> _transform;
		//osg::ref_ptr<osg::Geode> _geode;

		float _minSize;
		float _maxSize;

		Direction _direction;
		OverflowBehavior _behavior;

		std::map<UIElement*, std::pair<osg::Vec3f, osg::Vec3f> > _childBoxes;
	};
}

#endif
