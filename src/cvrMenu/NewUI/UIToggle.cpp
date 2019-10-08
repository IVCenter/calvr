#include "cvrMenu/NewUI/UIToggle.h"

using namespace cvr;

bool UIToggle::onButtonPress(bool pressed)
{
	if (pressed)
	{
		_on = !_on;
		return onToggle();
	}
	return false;
}