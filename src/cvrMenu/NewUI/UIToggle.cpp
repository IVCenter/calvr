#include "cvrMenu/NewUI/UIToggle.h"

using namespace cvr;

bool UIToggle::onButtonPress(bool pressed)
{
	if (pressed)
	{
		toggle();
	}
	return false;
}

bool UIToggle::toggle()
{
	_on = !_on;
	return onToggle();
}