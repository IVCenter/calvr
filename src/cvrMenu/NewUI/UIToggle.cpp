#include "cvrMenu/NewUI/UIToggle.h"

using namespace cvr;

bool UIToggle::onButtonPress(bool pressed)
{
	_on = !_on;
	return onToggle();
}