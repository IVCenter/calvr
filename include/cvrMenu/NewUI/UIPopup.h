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

		osg::Vec3 getPos() { return _menuPos; }

		virtual void setActive(bool active = true, bool attachToScene = true);

		virtual bool isActive();

		virtual void setPosition(osg::Vec3 pos);


		virtual void setRotation(osg::Quat rot);

	protected:

		virtual bool init();

		virtual void updateStart();

		virtual bool processEvent(InteractionEvent* event);

		virtual bool processIsect(IsectInfo& isect, int hand);

		virtual void updateEnd();

		virtual void itemDelete(MenuItem* item);

		virtual void orientTowardsHead();

		virtual void processMovement(TrackedButtonInteractionEvent* tie);

		osg::ref_ptr<osg::MatrixTransform> _menuRoot;
		UIElement* _rootElement;
		bool _menuActive;

		UIElement* _activeElement;
		bool _foundItem; // Has the isect found anything this frame?
		bool _interacting; // Is the last found active element being interacted with currently?
		bool _grabbable;
		int _grabButton = 1; // The button for moving menus around
		std::map<int, osg::Vec3> _currentPoint; // The world space point each hand is currently intersecting with
		float _moveDistance;

		osg::Vec3 _menuPos;
		osg::Quat _menuRot;
	};

}

#endif