#include "cvrMenu/NewUI/UIElement.h"
#include "cvrMenu/NewUI/UIUtil.h"
#include <algorithm>

using namespace cvr;

UIElement::UIElement()
{
	_percentPos = osg::Vec3(0, 0, 0);
	_percentSize = osg::Vec3(1, 1, 1);
	_absolutePos = osg::Vec3(0, -0.1f, 0);
	_absoluteSize = osg::Vec3(0, 0, 0);

	_aspect = osg::Vec3(1, 0, 1);
	_useAspect = false;
	_alignment = NONE;

	_actualPos = osg::Vec3(0, 0, 0);
	_actualSize = osg::Vec3(0, 0, 0);

	_children = std::vector<std::shared_ptr<UIElement> >();
	_parent = nullptr;

	_group = new osg::Group();
	_intersect = NULL;
	_dirty = true;
	_handle = false;
	_lastHand = -1;
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
		calculateBounds(pos, size);

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

void UIElement::calculateBounds(osg::Vec3 pos, osg::Vec3 size)
{
	_actualPos = pos + UIUtil::multiplyComponents(size, _percentPos) + _absolutePos;
	_actualSize = UIUtil::multiplyComponents(size, _percentSize) + _absoluteSize;
	osg::Vec3 targetSize = _actualSize;

	if (_useAspect)
	{
		float xmult, ymult, zmult;
		xmult = abs(_aspect.x()) > 1e-8 ? _actualSize.x() / _aspect.x() : 1e10;
		ymult = abs(_aspect.y()) > 1e-8 ? _actualSize.y() / _aspect.y() : 1e10;
		zmult = abs(_aspect.z()) > 1e-8 ? _actualSize.z() / _aspect.z() : 1e10;

		float mult = std::min(xmult, std::min(ymult, zmult));
		_actualSize = _aspect * mult;
	}

	//Can't let the scale of anything be truly zero (otherwise triangleintersect starts going 'SCREW U BUDDY U CANT DO THAT')
	_actualSize.x() = abs(_actualSize.x()) < 1e-6 ? 1e-6 : _actualSize.x();
	_actualSize.y() = abs(_actualSize.y()) < 1e-6 ? 1e-6 : _actualSize.y();
	_actualSize.z() = abs(_actualSize.z()) < 1e-6 ? 1e-6 : _actualSize.z();

	if (_alignment != NONE && _alignment != LEFT_TOP)
	{
		osg::Vec3 diff = targetSize - _actualSize;
		if (diff.length2() < 0.1f)
		{
			return;
		}

		switch (getAlign())
		{
		case LEFT_CENTER:
			_actualPos.z() -= diff.z() / 2.0f;
			break;
		case LEFT_BOTTOM:
			_actualPos.z() -= diff.z();
			break;
		case CENTER_TOP:
			_actualPos.x() += diff.x() / 2.0f;
			break;
		case CENTER_CENTER:
			_actualPos.x() += diff.x() / 2.0f;
			_actualPos.z() -= diff.z() / 2.0f;
			break;
		case CENTER_BOTTOM:
			_actualPos.x() += diff.x() / 2.0f;
			_actualPos.z() -= diff.z();
			break;
		case RIGHT_TOP:
			_actualPos.x() += diff.x();
			break;
		case RIGHT_CENTER:
			_actualPos.x() += diff.x();
			_actualPos.z() -= diff.z() / 2.0f;
			break;
		case RIGHT_BOTTOM:
			_actualPos.x() += diff.x();
			_actualPos.z() -= diff.z();
			break;
		default:
			break;
		}
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
		_lastHand = hand;
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
	//std::shared_ptr<UIElement> e2 = std::shared_ptr<UIElement>(e);
	/*if (e->_parent)
	{
		e->_parent->removeChild(e);
	}*/
	if (e->_parent != this) {
		e->_parent = this;

		_children.push_back(std::shared_ptr<UIElement>(e));
		_group->addChild(e->_group);
	}
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

UIElement* UIElement::getChild(int index)
{
	if (index >= 0 && index < _children.size())
	{
		return _children[index].get();
	}
	return NULL;
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

void UIElement::setAspect(osg::Vec3 aspect, bool useAspect)
{
	if (_useAspect != useAspect)
	{
		_useAspect = useAspect;
		_dirty = true;
	}
	if (_aspect != aspect)
	{
		_aspect = aspect;
		_dirty = true;
	}
}

void UIElement::setAlign(Alignment align)
{
	if (_alignment != align)
	{
		_alignment = align;
		_dirty = true;
	}
}

void UIElement::dummy(int dummy)
{
	dummy = 3;
}

