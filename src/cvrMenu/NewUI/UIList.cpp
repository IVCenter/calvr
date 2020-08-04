#include "cvrMenu/NewUI/UIList.h"
#include "cvrMenu/NewUI/UIUtil.h"
#include "cvrKernel/NodeMask.h"


using namespace cvr;

void UIList::updateElement(osg::Vec3 pos, osg::Vec3 size)
{
	if (_dirty)
	{
		calculateBounds(pos, size);
		
		int line = 0;
		int pos = 0;
		float max = 0;

		float size = 0;
		float spacing = 0;

		osg::Vec3 stepvec = osg::Vec3(0, 0, 0);
		osg::Vec3 sizevec = osg::Vec3(0, 0, 0);
		osg::Vec3 linevec = osg::Vec3(0, 0, 0);

		if (_direction == Direction::LEFT_TO_RIGHT)
		{
			//Calculate spacing between elements and total amount of empty space needed
			spacing = _actualSize.x() * _percentSpacing + _absoluteSpacing;
			float totalSpacing = spacing * (_children.size() - 1);

			//Space per element = total non-empty space / num children, clamped to be between min and max size
			size = (_actualSize.x() - totalSpacing) / _children.size();
			size = osg::clampAbove(size, _minSize);
			size = osg::clampBelow(size, _maxSize);

			max = _actualSize.x();

			//Number of lines (if more than one line needed)
			float numlines = ceil((size * _children.size()) / max);

			stepvec = osg::Vec3(size + spacing, 0, 0);
			sizevec = osg::Vec3(size, _actualSize.y(), _actualSize.z());
			if (_behavior == OverflowBehavior::WRAP)
			{
				sizevec.z() = _actualSize.z() / numlines;
				linevec.z() = -_actualSize.z() / numlines;
			}
			else if (_behavior == OverflowBehavior::WRAP_REVERSE)
			{
				sizevec.z() = _actualSize.z() / numlines;
				linevec.z() = _actualSize.z() / numlines;
			}
		}

		else if (_direction == Direction::TOP_TO_BOTTOM)
		{
			//Calculate spacing between elements and total amount of empty space needed
			spacing = _actualSize.z() * _percentSpacing + _absoluteSpacing;
			float totalSpacing = spacing * (_children.size() - 1);

			//Space per element = total non-empty space / num children, clamped to be between min and max size
			size = (_actualSize.z() - totalSpacing) / _children.size();
			size = osg::clampAbove(size, _minSize);
			size = osg::clampBelow(size, _maxSize);

			max = _actualSize.z();

			//Number of lines (if more than one line needed)
			float numlines = floor((size * _children.size()) / max);

			//After each element step down by size + spacing
			stepvec = osg::Vec3(0, 0, -(size + spacing));
			sizevec = osg::Vec3(_actualSize.x(), _actualSize.y(), size);
			if (_behavior == OverflowBehavior::WRAP)
			{
				sizevec.x() = _actualSize.x() / numlines;
				linevec.x() = _actualSize.x() / numlines;
			}
			else if (_behavior == OverflowBehavior::WRAP_REVERSE)
			{
				sizevec.z() = _actualSize.x() / numlines;
				linevec.x() = -_actualSize.x() / numlines;
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
			_children[i]->setDirty(true);
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
	this->setDirty(true);
}

void UIList::removeChild(UIElement* e)
{
	if (_childBoxes.find(e) != _childBoxes.end())
	{
		_childBoxes.erase(e);
	}
	UIElement::removeChild(e);
	this->setDirty(true);
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

void UIList::setSpacing(float absoluteSpacing, float percentSpacing)
{
	setAbsoluteSpacing(absoluteSpacing);
	setPercentSpacing(percentSpacing);
}

void UIList::setAbsoluteSpacing(float absoluteSpacing)
{
	if (_absoluteSpacing != absoluteSpacing)
	{
		_absoluteSpacing = absoluteSpacing;
		_dirty = true;
	}
}

void UIList::setPercentSpacing(float percentSpacing)
{
	if (_percentSpacing != percentSpacing)
	{
		_percentSpacing = percentSpacing;
		_dirty = true;
	}
}