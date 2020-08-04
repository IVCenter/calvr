#include "cvrMenu/NewUI/UIMenu.h"

using namespace cvr;

UIMenu::UIMenu()
{
	_scale = 1;
	_movable = true;
}

UIMenu::~UIMenu()
{

}

void UIMenu::updateStart()
{

}

void UIMenu::updateEnd()
{

}

bool UIMenu::processEvent(InteractionEvent* event)
{
	return false;
}

bool UIMenu::processIsect(IsectInfo & isect, int hand)
{
	return false;
}

void UIMenu::clear()
{

}

void UIMenu::close()
{

}

void UIMenu::setScale(float scale)
{
	_scale = scale;
}

float UIMenu::getScale()
{
	return _scale;
}

void UIMenu::itemDelete(MenuItem* item)
{

}

void UIMenu::setMenu(SubMenu* menu)
{

}