#include "cvrMenu/NewUI/UIElement.h"
#include "cvrMenu/NewUI/UIUtil.h"

using namespace cvr;

UIElement::UIElement()
{
	_percentPos = osg::Vec3(0, 0, 0);
	_percentSize = osg::Vec3(1, 1, 1);
	_absolutePos = osg::Vec3(0, -0.01f, 0);
	_absoluteSize = osg::Vec3(0, 0, 0);

	_actualPos = osg::Vec3(0, 0, 0);
	_actualSize = osg::Vec3(0, 0, 0);

	_children = std::vector<std::shared_ptr<UIElement> >();

	_group = new osg::Group();
	_intersect = NULL;
	_dirty = true;

}

UIElement::~UIElement()
{
	_children.clear();
}

void UIElement::updateElement(osg::Vec3 pos, osg::Vec3 size)
{
	if (!_intersect)
	{
		createGeometry();
	}
	if (_dirty)
	{
		_actualPos = pos + UIUtil::multiplyComponents(size, _percentPos) + _absolutePos;
		_actualSize = UIUtil::multiplyComponents(size, _percentSize) + _absoluteSize;

		updateGeometry();
		for (int i = 0; i < _children.size(); ++i)
		{
			_children[i]->setDirty(true);
		}
		_dirty = false;
	}

	for (int i = 0; i < _children.size(); ++i)
	{
		_children[i]->updateElement(_actualPos, _actualSize);
	}
}

void UIElement::createGeometry()
{
	_intersect = new osg::Geode();
}

void UIElement::updateGeometry()
{

}

UIElement* UIElement::processIsect(IsectInfo & isect, int hand)
{
	if (_intersect.valid() && isect.geode == _intersect.get())
	{
		_lastHitPoint = isect.localpoint;
		return this;
	}
	else
	{
		for (int i = 0; i < _children.size(); ++i)
		{
			UIElement* e = _children[i]->processIsect(isect, hand);
			if (e)
			{
				return e;
			}
		}
	}
	return NULL;
}

void UIElement::addChild(UIElement* e)
{
	_children.push_back(std::shared_ptr<UIElement>(e));
	_group->addChild(e->_group);
}

void UIElement::removeChild(UIElement* e)
{
	_group->removeChild(e->_group);
	for (int i = 0; i < _children.size(); ++i)
	{
		if (_children[i].get() == e)
		{
			_children.erase(_children.begin() + i);
			break;
		}
	}
}

void UIElement::setPercentPos(osg::Vec3 pos)
{
	if (_percentPos != pos)
	{
		_percentPos = pos;
		_dirty = true;
	}
}

void UIElement::setAbsolutePos(osg::Vec3 pos)
{
	if (_absolutePos != pos)
	{
		_absolutePos = pos;
		_dirty = true;
	}
}

void UIElement::setPercentSize(osg::Vec3 size)
{
	if (_percentSize != size)
	{
		_percentSize = size;
		_dirty = true;
	}
}

void UIElement::setAbsoluteSize(osg::Vec3 size)
{
	if (_absoluteSize != size)
	{
		_absoluteSize = size;
		_dirty = true;
	}
}