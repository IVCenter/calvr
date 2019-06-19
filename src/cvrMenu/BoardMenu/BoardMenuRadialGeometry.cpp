#include <cvrMenu/BoardMenu/BoardMenuRadialGeometry.h>
#include <cvrUtil/Bounds.h>
#include <cvrInput/TrackingManager.h>

#include <osg/Geometry>
#include <osg/Version>

#define MIN_CENTER 0.2f
#define MAX_CENTER 0.3f
#define MIN_RADIUS 150.0f
#define CIRCLE_STEPS 200
#define LIFT_DISTANCE 10.0f

using namespace cvr;

BoardMenuRadialGeometry::BoardMenuRadialGeometry() :
        BoardMenuGeometry()
{
	_lastX = 0;
	_lastY = 0;
	_center = 0;
	_radius = 0;
	_hoverIndex = -1;
	_selectedIndex = 0;
}

BoardMenuRadialGeometry::~BoardMenuRadialGeometry()
{
}

void BoardMenuRadialGeometry::selectItem(bool on)
{
}

void BoardMenuRadialGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
	_group = new osg::Group();
    _intersect = new osg::Geode();
	_selection = new osg::MatrixTransform();
    _node->addChild(_intersect);
    _node->addChild(_group);
    _item = item;

    _radial = dynamic_cast<MenuRadial*>(item);
	_selectedIndex = _radial->getValue();


	_outerArcs = std::vector<osg::ref_ptr<osg::MatrixTransform>>();
	_innerArcs = std::vector<osg::ref_ptr<osg::Geometry>>();
	_text = std::vector<osg::ref_ptr<osgText::Text>>();

	generateRadial();
}

void BoardMenuRadialGeometry::generateRadial()
{
	osg::Geode* geode = new osg::Geode();
	_group->addChild(geode);

	std::vector<std::string> labels = _radial->getLabels();
	float step = 2.0f * osg::PIf / (float)(labels.size());


	//find maximum radius of circle encapsulating text, while also generating text geometry
	float rmax = 0;
	for (int i = 0; i < labels.size(); ++i)
	{
		float theta = step * (float)(i);

		osgText::Text* text = makeText(labels[i], _textSize, osg::Vec3(0, 0, 0), _textColor, osgText::Text::AlignmentType::CENTER_CENTER);
		osg::BoundingBox bb = cvr::getBound(text);
		float r = bb.xMax() - bb.xMin();
		r = r * 0.8f;
		rmax = r > rmax ? r : rmax;

		_text.push_back(text);
	}

	//Find minimum circle radius that can hold text in each arc
	_radius = rmax * ((1.0 + sin(step / 2.0)) / (sin(step / 2.0)));
	float minrad = MIN_RADIUS * sqrt(((float)(labels.size()) / 4.0f));
	_radius = minrad > _radius ? minrad : _radius;

	//If minimum circle radius would mean having a center section smaller than the minimum, increase the circle radius
	_center = 1.0 - ((2 * rmax) / _radius);
	if (_center < MIN_CENTER)
	{
		_center = MIN_CENTER;
		_radius = (2.0 * rmax) / (1.0 - MIN_CENTER);
	}
	else if (_center > MAX_CENTER)
	{
		_center = MAX_CENTER;
	}

	osg::Geometry* selectionCircle = makeArc(0.0f, 1.0f, _radius / 20.0f, _radius / 20.0f, 0.0f, 2.0f * osg::PIf, CIRCLE_STEPS, osg::Vec4(1.0, 1.0, 1.0, 1.0), osg::Vec3(_radius, -(LIFT_DISTANCE + 3.0f), -_radius));
	osg::Geode* selectionGeode = new osg::Geode();
	selectionGeode->addDrawable(selectionCircle);
	_selection->addChild(selectionGeode);
	_node->addChild(_selection);



	//Set the positions of the text
	float centerRad = _radius * (0.5f + (_center / 2.0f));
	for (int i = 0; i < labels.size(); ++i)
	{
		float theta = step * (float)(i)+(step * 0.5f);
		_text[i]->setPosition(osg::Vec3((cos(theta) * centerRad) + _radius, -3, (sin(theta) * centerRad) - _radius));
	}

	for (int i = 0; i < labels.size(); ++i) {
		float theta = step * (float)(i);

		osg::Geometry* innerSlice;
		if (i == _selectedIndex)
		{
			innerSlice = makeArc(0, _center, _radius, _radius, theta, theta + step, CIRCLE_STEPS / labels.size(), osg::Vec4(0.4, 0.8, 0.4, 1.0), osg::Vec3(_radius, -2, -_radius));
		}
		else
		{
			innerSlice = makeArc(0, _center, _radius, _radius, theta, theta + step, CIRCLE_STEPS / labels.size(), osg::Vec4(0.8, 0.8, 0.8, 1.0), osg::Vec3(_radius, -2, -_radius));
		}
		geode->addDrawable(innerSlice);
		_innerArcs.push_back(innerSlice);


		osg::Geometry* outerSlice = makeArc(_center, 1.0f, _radius, _radius, theta, theta + step, CIRCLE_STEPS / labels.size(), osg::Vec4(0.3, 0.3, 0.3, 1.0), osg::Vec3(_radius, -2, -_radius));
		osg::Geode* outerGeode = new osg::Geode();
		osg::MatrixTransform* outerTransform = new osg::MatrixTransform();
		outerGeode->addDrawable(outerSlice);
		outerTransform->addChild(outerGeode);
		outerTransform->addChild(_text[i]);
		_group->addChild(outerTransform);
		_outerArcs.push_back(outerTransform);

		osg::Geometry* lineSegment = makeLine(osg::Vec3(_radius, -3, -_radius), osg::Vec3(_radius + cos(theta) * _radius, -3, -_radius + sin(theta) * _radius), osg::Vec4(0.0, 0.0, 0.0, 1.0));
		geode->addDrawable(lineSegment);
	}
	_width = _radius * 2.0;
	_height = _radius * 2.0;
}

void BoardMenuRadialGeometry::updateGeometry()
{
	std::vector<std::string> labels = _radial->getLabels();
	bool changedText = false;
	if (_text.size() == labels.size())
	{
		for (int i = 0; i < _text.size(); ++i) {
			if(_text[i]->getText().createUTF8EncodedString().compare(labels[i]) != 0)
			{
				changedText = true;
			}
		}
	}
	else
	{
		changedText = true;
	}

	if (changedText)
	{
		_group->removeChildren(0, _group->getNumChildren());
		_selection->removeChildren(0, _selection->getNumChildren());
		_innerArcs.clear();
		_outerArcs.clear();
		_text.clear();

		generateRadial();
	}
}

void BoardMenuRadialGeometry::update(osg::Vec3 & pointerStart, osg::Vec3 & pointerEnd)
{
	_selection->setMatrix(osg::Matrix::translate(osg::Vec3(_lastX * _radius, 0, _lastY * _radius)));
}

void BoardMenuRadialGeometry::processEvent(InteractionEvent * event)
{
	ValuatorInteractionEvent * val = event->asValuatorEvent();
    if(val)
    {
		ValuatorInteractionEvent * val = event->asValuatorEvent();
		if (val->getValuator() == 0)
		{
			_lastX = val->getValue();
		}
		else if (val->getValuator() == 1)
		{
			_lastY = val->getValue();
		}
    }

	double rad = atan2(_lastY, _lastX);
	rad = rad >= 0 ? rad : rad + 2.0 * osg::PI;
	double length = osg::Vec2(_lastX, _lastY).length();
	std::vector<std::string> labels = _radial->getLabels();
	double step = 2.0 * osg::PI / (double)(labels.size());
	int ind = (int)(rad / step);

	if (ind < 0 || ind >= _outerArcs.size())
	{
		return;
	}

	if (length > _center && _hoverIndex != ind)
	{
		if (_hoverIndex >= 0 && _hoverIndex < labels.size()) 
		{
			_outerArcs[_hoverIndex]->setMatrix(osg::Matrix::identity());
		}
		_outerArcs[ind]->setMatrix(osg::Matrix::translate(osg::Vec3(0, -LIFT_DISTANCE, 0)));

		_hoverIndex = ind;
	}
	else if(length <= _center && _hoverIndex != -1)
	{
		_outerArcs[_hoverIndex]->setMatrix(osg::Matrix::identity());

		_hoverIndex = -1;
	}


	TrackedButtonInteractionEvent * tie = event->asTrackedButtonEvent();
	if (tie)
	{
		if (tie->getButton() == 3 && event->getInteraction() == BUTTON_DOWN)
		{
			if (length > _center)
			{
				if (ind != _selectedIndex)
				{
					osg::Vec4Array* colors = new osg::Vec4Array;
					colors->push_back(osg::Vec4(0.8, 0.8, 0.8, 1.0));
					_innerArcs[_selectedIndex]->setColorArray(colors);
					_innerArcs[_selectedIndex]->setColorBinding(osg::Geometry::AttributeBinding::BIND_OVERALL);

					colors = new osg::Vec4Array;
					colors->push_back(osg::Vec4(0.4, 0.8, 0.4, 1.0));
					_innerArcs[ind]->setColorArray(colors);
					_innerArcs[ind]->setColorBinding(osg::Geometry::AttributeBinding::BIND_OVERALL);

					_selectedIndex = ind;
					_radial->setValue(ind);
				}
				if (_radial->getCallback())
				{
					_radial->getCallback()->menuCallback(_radial, event->asHandEvent() ? event->asHandEvent()->getHand() : 0);
				}
			}
		}
	}

}
