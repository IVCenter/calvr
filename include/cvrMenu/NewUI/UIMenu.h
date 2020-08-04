#ifndef CALVR_UI_MENU_H
#define CALVR_UI_MENU_H

#include <cvrMenu/MenuBase.h>

#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Geode>

namespace cvr
{

	class UIMenu : public MenuBase
	{
	public:
		UIMenu();
		virtual ~UIMenu();

		virtual void setMenu(SubMenu* menu);

		virtual void updateStart();

		virtual bool processEvent(InteractionEvent* event);

		virtual bool processIsect(IsectInfo & isect, int hand);

		virtual void updateEnd();

		virtual void itemDelete(MenuItem* item);

		virtual void clear();

		virtual void close();

		virtual void setScale(float scale);

		virtual float getScale();

		virtual void setMovable(bool m)
		{
			_movable = m;
		}

	protected:
		bool _movable;
		float _scale;
	};

}

#endif