#include "cvrMenu/NewUI/UIQuadElement.h"
#include "cvrMenu/NewUI/UIUtil.h"
#include "cvrKernel/NodeMask.h"

using namespace cvr;

void UIQuadElement::createGeometry()
{
	_transform = new osg::MatrixTransform();
	_intersect = new osg::Geode();

	_group->addChild(_transform);
	//_transform->addChild(_intersect);
	_transform->addChild(_geode);

	_intersect->setNodeMask(cvr::INTERSECT_MASK);

	osg::Geometry* drawable = UIUtil::makeQuad(1, 1, _color, osg::Vec3(0,0,0));
	_geode->addDrawable(drawable);
	//should quadelement have an intersectable?
	//_intersect->addDrawable(drawable);

	updateGeometry();
}

void UIQuadElement::updateGeometry()
{
	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(_color);
	((osg::Geometry*)_geode->getDrawable(0))->setColorArray(colors, osg::Array::BIND_OVERALL);
	((osg::Geometry*)_geode->getDrawable(0))->setVertexAttribArray(2, colors, osg::Array::BIND_OVERALL);

	osg::Matrix mat = osg::Matrix();
	mat.makeScale(_actualSize);
	mat.postMultTranslate(_actualPos);
	_transform->setMatrix(mat);
}

void UIQuadElement::setColor(osg::Vec4 color)
{
	if (_color != color)
	{
		_color = color;
		_dirty = true;
		/*
		//No need to update children / set dirty when only color changes
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(_color);
		((osg::Geometry*)_geode->getDrawable(0))->setColorArray(colors, osg::Array::BIND_OVERALL);
		*/
	}
}

void UIQuadElement::setTransparent(bool transparent)
{
	if (transparent)
	{
		_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	}
	else
	{
		_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
		_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::OFF);
	}
}

void UIQuadElement::setRounding(float absRounding, float percentRounding)
{
	_absoluteRounding->set(absRounding);
	_percentRounding->set(percentRounding);
}
void UIQuadElement::setBorderSize(float size) {	// 0 = no border, .5 is half the quad
	_borderSizeUniform->set(size);
}

void UIQuadElement::borderOnly(bool borderOnly) {
	_borderOnlyUniform->set(borderOnly);
}