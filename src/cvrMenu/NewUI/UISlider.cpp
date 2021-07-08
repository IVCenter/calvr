#include "cvrMenu/NewUI/UISlider.h"
#include "cvrKernel/NodeMask.h"
#include "cvrInput/TrackingManager.h"

#include <algorithm>

using namespace cvr;


UISlider::UISlider(std::string emptytexture, std::string filledtexture, std::string handletexture)
	: UIElement()
{
	empty = new UITexture(emptytexture);
	filled = new UITexture(filledtexture);
	handle = new UITexture(handletexture);
	handle->setAbsolutePos(handle->getAbsolutePos() + osg::Vec3(0, -0.1f, 0));
	addChild(empty);
	addChild(filled);
	addChild(handle);
	_button = 0;
	_percent = 1e-9;
	_held = false;
}

UISlider::UISlider(osg::Vec4 emptyColor, osg::Vec4 filledColor, osg::Vec4 handleColor)
	: UIElement()
{
	empty = new UITexture(emptyColor);
	filled = new UITexture(filledColor);
	handle = new UITexture(handleColor);
	handle->setAbsolutePos(handle->getAbsolutePos() + osg::Vec3(0, -0.1f, 0));
	addChild(empty);
	addChild(filled);
	addChild(handle);
	_button = 0;
	_percent = 1e-9;
	_held = false;
}

void UISlider::updateElement(osg::Vec3 pos, osg::Vec3 size)
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


		_emptywidth = _actualSize.x() * (1.0f - _percent);
		_filledwidth = _actualSize.x() * _percent;
	}

	for (int i = 0; i < _children.size(); ++i)
	{
		UIElement* child = _children[i].get();

		if (child == filled)
		{
			filled->updateElement(_actualPos, osg::Vec3(_filledwidth, _actualSize.y(), _actualSize.z()));
		}
		else if (child == empty)
		{
			empty->updateElement(_actualPos + osg::Vec3(_filledwidth, 0, 0), osg::Vec3(_emptywidth, _actualSize.y(), _actualSize.z()));
		}
		else if (child == handle)
		{
			handle->updateElement(_actualPos + osg::Vec3(_filledwidth, 0, 0), _actualSize);
		}
		else
		{
			_children[i]->updateElement(_actualPos, _actualSize);
		}
	}
}

void UISlider::createGeometry()
{
	_transform = new osg::MatrixTransform();
	_intersect = new osg::Geode();
	//_geode = new osg::Geode();

	_group->addChild(_transform);
	_transform->addChild(_intersect);
	//_transform->addChild(_geode);

	_intersect->setNodeMask(cvr::INTERSECT_MASK);

	osg::Geometry* drawable = UIUtil::makeQuad(1, 1, osg::Vec4(1,1,1,1), osg::Vec3(0, 0, 0));
	//_geode->addDrawable(drawable);
	//should quadelement have an intersectable?
	_intersect->addDrawable(drawable);

	updateGeometry();

}

void UISlider::updateGeometry()
{
	osg::Matrix mat = osg::Matrix();
	mat.makeScale(_actualSize);
	mat.postMultTranslate(_actualPos);
	_transform->setMatrix(mat);
}

void UISlider::setPercent(float p)
{
	p = std::min(std::max(1e-9f, p), 1.0f - 1e-9f);
	if (_percent != p)
	{
		_percent = p;
		_dirty = true;
	}
}

bool UISlider::processEvent(InteractionEvent* event)
{
	TrackedButtonInteractionEvent* tie = event->asTrackedButtonEvent();
	if (tie && tie->getButton() == _button)
	{
		if (tie->getInteraction() == BUTTON_DOWN)
		{
			_held = true;
			return true;
		}
		else if (tie->getInteraction() == BUTTON_DRAG && _held)
		{
			osg::MatrixList ltw = _intersect->getWorldMatrices();
			osg::Matrix m = ltw[0];//osg::Matrix::identity();
			//for (int i = 0; i < ltw.size(); ++i)
			//{
			//	m.postMult(ltw[i]);
			//}
			osg::Matrix mi = osg::Matrix::inverse(m);

			osg::Vec4d l4 = osg::Vec4(0, 1, 0, 0) * TrackingManager::instance()->getHandMat(tie->getHand());
			osg::Vec3 l = osg::Vec3(l4.x(), l4.y(), l4.z());

			osg::Vec4d l04 = osg::Vec4(0, 0, 0, 1) * TrackingManager::instance()->getHandMat(tie->getHand());
			osg::Vec3 l0 = osg::Vec3(l04.x(), l04.y(), l04.z());

			osg::Vec4d n4 = osg::Vec4(0, 1, 0, 0) * m;
			osg::Vec3 n = osg::Vec3(n4.x(), n4.y(), n4.z());

			osg::Vec4d p04 = osg::Vec4(0, 0, 0, 1) * m;
			osg::Vec3 p0 = osg::Vec3(p04.x(), p04.y(), p04.z());


			osg::Vec3 p = l0 + l * (((p0 - l0) * n) / (l * n));

			osg::Vec4 pl = osg::Vec4(p.x(), p.y(), p.z(), 1) * mi;

			setPercent(pl.x());
			//_percent = pl.x(); // _lastHitPoint.x();
			//_dirty = true;
			//std::cerr << "<" << _lastHitPoint.x() << ", " << _lastHitPoint.y() << ", " << _lastHitPoint.z() << ">" << std::endl;
			onPercentChange();
			return true;
		}
		else if (tie->getInteraction() == BUTTON_UP)
		{
			_held = false;
			return false;
		}
	}

	return _held;
}

void UISlider::processHover(bool hover)
{

}