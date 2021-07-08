#include "cvrMenu/NewUI/UIRadial.h"
#include <algorithm>

using namespace cvr;

UIRadialButton::UIRadialButton(UIRadial* parent, int button)
	: UIButton(button)
{
	parent->addButton(this);
}

UIRadialButton::UIRadialButton(int button)
	: UIButton(button)
{
	_parent = nullptr;
}

bool UIRadialButton::onButtonPress(bool pressed)
{
	if (pressed && _parent.get())
	{
		return _parent->onButtonPress(this);
	}
	return false;
}

UIRadial* UIRadialButton::getRadial()
{
	return _parent.get();
}

void UIRadialButton::setRadial(UIRadial* r)
{
	_parent = std::shared_ptr<UIRadial>(r);
}



UIRadial::UIRadial()
{
	_current = _allowNoneSelected ? -1 : 0;
	_buttons = std::vector<UIRadialButton*>();
}

void UIRadial::addButton(UIRadialButton* b)
{
	_buttons.push_back(b);
	b->setRadial(this);
}

void UIRadial::removeButton(UIRadialButton* b)
{
	std::vector<UIRadialButton*>::iterator it = std::find(_buttons.begin(), _buttons.end(), b);
	if (it != _buttons.end())
	{
		_buttons.erase(it);
		b->setRadial(nullptr);
	}
}

bool UIRadial::onButtonPress(UIRadialButton* b)
{
	for (int i = 0; i < _buttons.size(); ++i)
	{
		if (_buttons[i] == b)
		{
			if (i == _current && _allowNoneSelected)
			{
				setCurrent(-1);
			}
			else if (i != _current)
			{
				setCurrent(i);
			}
		}
	}
	return true;
}

void UIRadial::setCurrent(int current)
{
	if (_current != current)
	{
		if (!_allowNoneSelected && current < 0)
		{
			return;
		}
		_current = current;
		onSelectionChange();
	}
}