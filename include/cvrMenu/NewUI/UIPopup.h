#ifndef CALVR_UI_POPUP_H
#define CALVR_UI_POPUP_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuSystemBase.h>
#include <cvrMenu/NewUI/UIElement.h>

#include <osg/Vec3>
#include <osg/Quat>

namespace cvr
{
	class CVRMENU_EXPORT UIPopup : public MenuSystemBase
	{
	public:
		UIPopup();
		virtual ~UIPopup();

		virtual void addChild(UIElement* e);

		virtual void removeChild(UIElement* e);

		virtual osg::MatrixTransform* getRoot() { return _menuRoot; }

		virtual UIElement* getRootElement() { return _rootElement; }

		virtual void setActive(bool active = true, bool attachToScene = true);

		virtual bool isActive();

		virtual void setPosition(osg::Vec3 pos);

	protected:

		virtual bool init();

		virtual void updateStart();

		virtual bool processEvent(InteractionEvent * event);

		virtual bool processIsect(IsectInfo & isect, int hand);

		virtual void updateEnd();

		virtual void itemDelete(MenuItem * item);

		virtual void orientTowardsHead();

		osg::ref_ptr<osg::MatrixTransform> _menuRoot;
		UIElement* _rootElement;
		bool _menuActive;

		UIElement* _activeElement;
		bool _foundItem; // Has the isect found anything this frame?
		bool _interacting; // Is the last found active element being interacted with currently?

		osg::Vec3 _menuPos;
		osg::Quat _menuRot;
	};

}

#endif