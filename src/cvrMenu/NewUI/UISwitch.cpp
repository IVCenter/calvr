#include "cvrMenu/NewUI/UISwitch.h"
#include "cvrMenu/NewUI/UIUtil.h"

using namespace cvr;

void UISwitch::updateElement(osg::Vec3 pos, osg::Vec3 size)
{
	if (!_intersect)
	{
		createGeometry();
	}
	if (_dirty)
	{
		calculateBounds(pos, size);

		for (int i = 0; i < _children.size(); ++i)
		{
			_children[i]->setDirty(true);
		}
		_dirty = false;
	}

	//Only update element that is currently being displayed
	if (_index > -1 && _index < _children.size())
	{
		_children[_index]->updateElement(_actualPos, _actualSize);
	}
}

void UISwitch::setDisplayed(int index)
{
	if (_index != index && index < _children.size())
	{
		UIElement* child;
		//If there is a currently displayed child, remove it
		if (child = getChild(_index))
		{
			_group->removeChild(child->getGroup());
		}
		//If a new child should be displayed, display it
		if (child = getChild(index))
		{
			_group->addChild(child->getGroup());
		}

		_dirty = true;
		_index = index;
	}
}

int UISwitch::getDisplayed()
{
	return _index;
}

void UISwitch::addChild(UIElement* e)
{
	_children.push_back(std::shared_ptr<UIElement>(e));
}