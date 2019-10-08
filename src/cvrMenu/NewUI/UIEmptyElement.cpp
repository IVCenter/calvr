#include "cvrMenu/NewUI/UIEmptyElement.h"
#include "cvrMenu/NewUI/UIUtil.h"


using namespace cvr;

UIEmptyElement::UIEmptyElement()
{

}

UIEmptyElement::~UIEmptyElement()
{

}

void UIEmptyElement::updateElement(osg::Vec3 pos, osg::Vec3 size)
{
	if (_dirty)
	{
		calculateBounds(pos, size);

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
