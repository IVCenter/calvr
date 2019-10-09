#include "cvrMenu/NewUI/UIText.h"
#include "cvrKernel/NodeMask.h"

using namespace cvr;


void UIText::createGeometry()
{
	_transform = new osg::MatrixTransform();
	_intersect = new osg::Geode();
	_geode = new osg::Geode();

	_group->addChild(_transform);
	_transform->addChild(_intersect);
	_transform->addChild(_geode);

	_intersect->setNodeMask(cvr::INTERSECT_MASK);

	_geode->addDrawable(_text);
	//Should text be intersectable? probably not
	//_intersect->addDrawable(_text);

	updateGeometry();
}

void UIText::updateGeometry()
{
	osg::Vec3 textPos = _actualPos;

	switch (getTextAlign()) 
	{
	case osgText::Text::LEFT_TOP:
		break;
	case osgText::Text::LEFT_CENTER:
		textPos.z() -= _actualSize.z() / 2;
		break;
	case osgText::Text::LEFT_BOTTOM:
		textPos.z() -= _actualSize.z();
		break;
	case osgText::Text::CENTER_TOP:
		textPos.x() += _actualSize.x() / 2;
		break;
	case osgText::Text::CENTER_CENTER:
		textPos.x() += _actualSize.x() / 2;
		textPos.z() -= _actualSize.z() / 2;
		break;
	case osgText::Text::CENTER_BOTTOM:
		textPos.x() += _actualSize.x() / 2;
		textPos.z() -= _actualSize.z();
		break;
	case osgText::Text::RIGHT_TOP:
		textPos.x() += _actualSize.x();
		break;
	case osgText::Text::RIGHT_CENTER:
		textPos.x() += _actualSize.x();
		textPos.z() -= _actualSize.z() / 2;
		break;
	case osgText::Text::RIGHT_BOTTOM:
		textPos.x() += _actualSize.x();
		textPos.z() -= _actualSize.z();
		break;
	}


	_text->setPosition(textPos);
	_text->setMaximumWidth(_actualSize.x());
	_text->setMaximumHeight(_actualSize.z());
}

osgText::Text::AlignmentType UIText::getTextAlign()
{
	return _text->getAlignment();
}

void UIText::setTextAlign(osgText::Text::AlignmentType alignment)
{
	if (getTextAlign() != alignment)
	{
		_text->setAlignment(alignment);
		_dirty = true;
	}
}

osgText::Font* UIText::getFont()
{
	return _text->getFont();
}

void UIText::setFont(osgText::Font* font)
{
	if (_text->getFont() != font)
	{
		_text->setFont(font);
		_dirty = true;
	}
}

const std::string& UIText::getText()
{
	return _text->getText().createUTF8EncodedString();
}

void UIText::setText(const std::string& text)
{
	if (getText().compare(text) != 0)
	{
		_text->setText(text);
		_dirty = true;
	}
}
