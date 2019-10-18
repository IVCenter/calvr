#include "cvrMenu/NewUI/UIPopup.h"
#include "cvrMenu/NewUI/UIEmptyElement.h"
#include "cvrKernel/SceneManager.h"
#include "cvrMenu/MenuManager.h"
#include "cvrInput/TrackingManager.h"

using namespace cvr;

UIPopup::UIPopup()
{
	_activeElement = NULL;
	_interacting = false;
	_foundItem = false;

	_menuRoot = new osg::MatrixTransform();
	_menuRoot->setMatrix(osg::Matrix::identity());
	_menuRoot->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	_rootElement = new UIEmptyElement();
	_rootElement->setAbsoluteSize(osg::Vec3(1000, 1, 1000));
	_menuRoot->addChild(_rootElement->getGroup());

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

bool UIPopup::processEvent(InteractionEvent * event)
{
	if (_activeElement)
	{
		_interacting = _activeElement->processEvent(event);
		return _interacting;
	}

	_interacting = false;
	return false;
}

bool UIPopup::processIsect(IsectInfo & isect, int hand)
{
	UIElement* e = _rootElement->processIsect(isect, hand);
	if (_interacting)
	{
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

void UIPopup::itemDelete(MenuItem * item)
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

void UIPopup::orientTowardsHead()
{
	osg::Vec3 menuPoint = _menuRoot->getMatrix().getTrans();
	//std::cerr << "move dist: " << _moveDistance << std::endl;

	//TODO: add hand/head mapping
	osg::Vec3 viewerPoint =
		TrackingManager::instance()->getHeadMat(0).getTrans();

	osg::Vec3 viewerDir = viewerPoint - menuPoint;
	viewerDir.z() = 0.0;

	osg::Matrix menuRot;
	menuRot.makeRotate(osg::Vec3(0, 1, 0), viewerDir);
	

	_menuRoot->setMatrix(menuRot);
}

void UIPopup::setPosition(osg::Vec3 pos)
{
	osg::Matrix menuPos;
	menuPos.makeTranslate(pos);

	_menuRoot->setMatrix(menuPos);
}