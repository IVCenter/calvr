#include "cvrMenu/NewUI/UIPopup.h"
#include "cvrMenu/NewUI/UIEmptyElement.h"
#include "cvrKernel/SceneManager.h"
#include "cvrMenu/MenuManager.h"
#include "cvrInput/TrackingManager.h"
#include "cvrUtil/OsgMath.h"

using namespace cvr;

UIPopup::UIPopup()
{
	_activeElement = NULL;
	_interacting = false;
	_foundItem = false;
	_grabbable = false;

	_menuRoot = new osg::MatrixTransform();
	_menuRoot->setMatrix(osg::Matrix::identity());
	_menuRoot->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	_menuPos = osg::Vec3(0, 0, 0);
	_menuRot = osg::Quat(0, 0, 0, 1);

	_rootElement = new UIEmptyElement();
	_rootElement->setAbsoluteSize(osg::Vec3(1000, 1, 1000));
	_menuRoot->addChild(_rootElement->getGroup());

	_currentPoint = std::map<int, osg::Vec3>();

	MenuManager::instance()->addMenuSystem(this);
}

UIPopup::~UIPopup()
{
	
}

bool UIPopup::init()
{
	return true;
}

void UIPopup::updateStart()
{
	//orientTowardsHead();
	_rootElement->updateElement(osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 0));
	_foundItem = false;
}

bool UIPopup::processEvent(InteractionEvent* event)
{
	if (_grabbable)
	{
		TrackedButtonInteractionEvent* tie = event->asTrackedButtonEvent();
		if (tie && tie->getButton() == _grabButton)
		{
			if (tie->getInteraction() == BUTTON_DOWN)
			{
				osg::Vec3 ray;
				ray = _currentPoint[tie->getHand()]
					- tie->getTransform().getTrans();

				if (!tie->asPointerEvent())
				{
					_moveDistance = ray.length();
				}
				else
				{
					_moveDistance = ray.y();
				}

			}
			else if (tie->getInteraction() == BUTTON_DRAG)
			{
				processMovement(tie);
			}
			return true;
		}
	}

	if (_activeElement)
	{
		_interacting = _activeElement->processEvent(event);
		return _interacting;
	}

	_interacting = false;
	return false;
}

bool UIPopup::processIsect(IsectInfo& isect, int hand)
{
	UIElement* e = _rootElement->processIsect(isect, hand);
	if (e)
	{
		_grabbable = e->getIsHandle();
		_currentPoint[hand] = isect.point;
	}
	if (_interacting)
	{
		return true;
		if (e)
		{
			return true;
		}
		return false;
	}
	else if (e)
	{
		if (_activeElement)
		{
			if (_activeElement != e)
			{
				e->processHover(true);
				_activeElement->processHover(false);
			}
		}
		else
		{
			e->processHover(true);
		}
		_activeElement = e;
		_foundItem = true;
		return true;
	}
	else
	{
		return false;
	}
}

void UIPopup::updateEnd()
{
	if (_activeElement && !_foundItem && !_interacting)
	{
		_activeElement->processHover(false);
		_activeElement = NULL;
	}
}

void UIPopup::itemDelete(MenuItem* item)
{

}

void UIPopup::addChild(UIElement* e)
{
	_rootElement->addChild(e);
}

void UIPopup::removeChild(UIElement* e)
{
	_rootElement->removeChild(e);
}

void UIPopup::setActive(bool active, bool attachToScene)
{
	if (active && attachToScene)
	{
		SceneManager::instance()->getMenuRoot()->addChild(_menuRoot);
	}
	else
	{
		std::vector<osg::Group*> parents = _menuRoot->getParents();
		for (int i = 0; i < parents.size(); ++i)
		{
			parents[i]->removeChild(_menuRoot);
		}
	}

	_menuActive = active;
}

bool UIPopup::isActive()
{
	return _menuActive;
}

void UIPopup::processMovement(TrackedButtonInteractionEvent* tie)
{
	if (!tie->asPointerEvent())
	{
		osg::Vec3 menuPoint = osg::Vec3(0, _moveDistance, 0);
		//std::cerr << "move dist: " << _moveDistance << std::endl;
		menuPoint = menuPoint * tie->getTransform();

		//TODO: add hand/head mapping
		osg::Vec3 viewerPoint =
			TrackingManager::instance()->getHeadMat(0).getTrans();

		osg::Vec3 viewerDir = viewerPoint - menuPoint;
		viewerDir.z() = 0.0;

		osg::Matrix menuRot;
		menuRot.makeRotate(osg::Vec3(0, -1, 0), viewerDir);

		_menuRoot->setMatrix(
			osg::Matrix::translate(-_menuPos) * menuRot
			* osg::Matrix::translate(menuPoint));
	}
	else
	{
		osg::Vec3 point1, point2(0, 1000, 0), planePoint, planeNormal(0, -1, 0),
			intersect;
		float w;
		point1 = point1 * tie->getTransform();
		point2 = point2 * tie->getTransform();
		planePoint = osg::Vec3(0,
			_moveDistance + tie->getTransform().getTrans().y(), 0);

		if (linePlaneIntersectionRef(point1, point2, planePoint, planeNormal,
			intersect, w))
		{
			_menuRoot->setMatrix(
				osg::Matrix::translate(intersect - _menuPos));
		}

	}
}

void UIPopup::orientTowardsHead()
{
	osg::Vec3 menuPoint = _menuRoot->getMatrix().getTrans();
	//std::cerr << "move dist: " << _moveDistance << std::endl;

	//TODO: add hand/head mapping
	osg::Vec3 viewerPoint =
		TrackingManager::instance()->getHeadMat(0).getTrans();

	osg::Vec3 viewerDir = viewerPoint - menuPoint;
	viewerDir.z() = 0.0;

	osg::Matrix menu;
	menu.makeRotate(osg::Vec3(0, -1, 0), viewerDir);
	_menuRot = menu.getRotate();
	std::cout << _menuRot.x() << ", " << _menuRot.y() << ", " << _menuRot.z() << ", " << _menuRot.w() << std::endl;
	menu.postMultTranslate(_menuPos);

	_menuRoot->setMatrix(menu);
}

void UIPopup::setPosition(osg::Vec3 pos)
{
	osg::Matrix menu;
	menu.makeTranslate(pos);
	_menuPos = pos;
	menu.preMultRotate(_menuRot);

	_menuRoot->setMatrix(menu);
}

void UIPopup::setRotation(osg::Quat rot)
{
	osg::Matrix menu;
	menu.makeTranslate(_menuPos);
	_menuRot = rot;
	menu.preMultRotate(_menuRot);

	_menuRoot->setMatrix(menu);

}