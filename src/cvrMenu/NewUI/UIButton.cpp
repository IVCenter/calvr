#include "cvrMenu/NewUI/UIButton.h"
#include "cvrMenu/NewUI/UIUtil.h"
#include "cvrKernel/NodeMask.h"

using namespace cvr;

void UIButton::createGeometry()
{
	_transform = new osg::MatrixTransform();
	_intersect = new osg::Geode();
	//_geode = new osg::Geode();

	_group->addChild(_transform);
	_transform->addChild(_intersect);
	//_transform->addChild(_geode);

	_intersect->setNodeMask(cvr::INTERSECT_MASK);

	osg::Geometry* drawable = UIUtil::makeQuad(1, 1, osg::Vec4(1, 1, 1, 1), osg::Vec3(0, 0, 0));
	//should button have a visible component?
	//_geode->addDrawable(drawable);

	_intersect->addDrawable(drawable);

	updateGeometry();
}

void UIButton::updateGeometry()
{
	osg::Matrix mat = osg::Matrix();
	mat.makeScale(_actualSize);
	mat.postMultTranslate(_actualPos);
	_transform->setMatrix(mat);
}

bool UIButton::processEvent(InteractionEvent * event)
{
	TrackedButtonInteractionEvent* tie = event->asTrackedButtonEvent();
	if (tie && tie->getButton() == _button)
	{
		if (tie->getInteraction() == BUTTON_DOWN && !_buttonDown)
		{
			_buttonDown = true;
			return onButtonPress(true);
		}
		else if (tie->getInteraction() == BUTTON_UP && _buttonDown)
		{
			_buttonDown = false;
			return onButtonPress(false);
		}
	}
	return false;
}

void UIButton::processHover(bool hover)
{
	if (!hover && _buttonDown)
	{
		_buttonDown = false;
		onButtonPress(false);
	}
}
