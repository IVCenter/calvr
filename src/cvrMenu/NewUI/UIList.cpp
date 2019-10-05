#include "cvrMenu/NewUI/UIList.h"
#include "cvrMenu/NewUI/UIUtil.h"
#include "cvrKernel/NodeMask.h"


using namespace cvr;

void UIList::updateElement(osg::Vec3 pos, osg::Vec3 size)
{
	if (_dirty)
	{
		_actualPos = pos + UIUtil::multiplyComponents(size, _percentPos) + _absolutePos;
		_actualSize = UIUtil::multiplyComponents(size, _percentSize) + _absoluteSize;
		
		int line = 0;
		int pos = 0;
		float max = 0;

		float size = 0;

		osg::Vec3 stepvec = osg::Vec3(0, 0, 0);
		osg::Vec3 sizevec = osg::Vec3(0, 0, 0);
		osg::Vec3 linevec = osg::Vec3(0, 0, 0);

		if (_direction == Direction::LEFT_TO_RIGHT)
		{
			size = _actualSize.x() / _children.size();
			size = osg::clampAbove(size, _minSize);
			size = osg::clampBelow(size, _maxSize);

			max = _actualSize.x();

			float linewidth = ceil((size * _children.size()) / max);

			stepvec = osg::Vec3(size, 0, 0);
			sizevec = osg::Vec3(size, _actualSize.y(), _actualSize.z());
			if (_behavior == OverflowBehavior::WRAP)
			{
				sizevec.z() = _actualSize.z() / linewidth;
				linevec.z() = -_actualSize.z() / linewidth;
			}
			else if (_behavior == OverflowBehavior::WRAP_REVERSE)
			{
				sizevec.z() = _actualSize.z() / linewidth;
				linevec.z() = _actualSize.z() / linewidth;
			}
		}

		else if (_direction == Direction::TOP_TO_BOTTOM)
		{
			size = _actualSize.z() / _children.size();
			size = osg::clampAbove(size, _minSize);
			size = osg::clampBelow(size, _maxSize);

			max = _actualSize.z();

			float linewidth = floor((size * _children.size()) / max);

			stepvec = osg::Vec3(0, 0, -size);
			sizevec = osg::Vec3(_actualSize.x(), _actualSize.y(), size);
			if (_behavior == OverflowBehavior::WRAP)
			{
				sizevec.x() = _actualSize.x() / linewidth;
				linevec.x() = _actualSize.x() / linewidth;
			}
			else if (_behavior == OverflowBehavior::WRAP_REVERSE)
			{
				sizevec.z() = _actualSize.x() / linewidth;
				linevec.x() = -_actualSize.x() / linewidth;
			}
		}

		for (int i = 0; i < _children.size(); ++i)
		{
			float dist = (pos+1) * size;
			if (dist > max)
			{
				if (_behavior == OverflowBehavior::CUT)
				{
					break;
				}
				else if (_behavior == OverflowBehavior::WRAP || _behavior == OverflowBehavior::WRAP_REVERSE)
				{
					++line;
					pos = 0;
				}
			}
			osg::Vec3 posvec = _actualPos + stepvec * pos + linevec * line;
			_childBoxes[_children[i].get()] = { posvec, sizevec };

			++pos;
		}

		_dirty = false;
	}

	for (int i = 0; i < _children.size(); ++i)
	{
		if (_children[i])
		{
			std::pair<osg::Vec3, osg::Vec3> box = _childBoxes[_children[i].get()];
			_children[i]->updateElement(box.first, box.second);
		}
	}
}

void UIList::addChild(UIElement* e)
{
	_childBoxes[e] = std::make_pair<osg::Vec3f, osg::Vec3f>(osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 0));
	UIElement::addChild(e);
}

void UIList::removeChild(UIElement* e)
{
	if (_childBoxes.find(e) != _childBoxes.end())
	{
		_childBoxes.erase(e);
	}
	UIElement::removeChild(e);
}

void UIList::setDirection(UIList::Direction direction)
{
	if (_direction != direction)
	{
		_direction = direction;
		_dirty = true;
	}
}

UIList::Direction UIList::getDirection()
{
	return _direction;
}

void UIList::setOverflow(UIList::OverflowBehavior behavior)
{
	if (_behavior != behavior)
	{
		_behavior = behavior;
		_dirty = true;
	}
}

UIList::OverflowBehavior UIList::getOverflow()
{
	return _behavior;
}

void UIList::setMinSize(float size)
{
	if (_minSize != size)
	{
		_minSize = size;
		_dirty = true;
	}
}

void UIList::setMaxSize(float size)
{
	if (_maxSize != size)
	{
		_maxSize = size;
		_dirty = true;
	}
}