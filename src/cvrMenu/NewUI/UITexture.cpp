#include "cvrMenu/NewUI/UITexture.h"
#include "cvrMenu/NewUI/UIUtil.h"
#include "cvrKernel/NodeMask.h"

using namespace cvr;

void UITexture::updateGeometry()
{
	UIQuadElement::updateGeometry();
	//_transform->setNodeMask(~cvr::INTERSECT_MASK);


	if (_texture.valid())
	{
		_geode->getDrawable(0)->getOrCreateStateSet()->setTextureAttributeAndModes(0, _texture, osg::StateAttribute::ON);
	}
}

void UITexture::setTexture(osg::Texture2D* texture)
{
	_texture = texture;
	/*
	if (_texture.valid())
	{
		_geode->getDrawable(0)->getOrCreateStateSet()->setTextureAttributeAndModes(0, _texture, osg::StateAttribute::ON);
	}
	*/
}

void UITexture::setTexture(std::string texturePath)
{
	_texture = UIUtil::loadImage(texturePath);
	/*
	if (_texture && _texture.valid())
	{
		_geode->getDrawable(0)->getOrCreateStateSet()->setTextureAttributeAndModes(0, _texture, osg::StateAttribute::ON);
	}
	*/
}
