#include <cvrMenu/MenuRadial.h>

#include <iostream>

using namespace cvr;

MenuRadial::MenuRadial() :
        MenuItem()
{
    _value = 0;
}

MenuRadial::~MenuRadial()
{
}

int MenuRadial::getValue() const
{
    return _value;
}

void MenuRadial::setValue(const int value)
{
    if(value < 0)
    {
        _value = 0;
    }
    else if(value >= _labels.size())
    {
        _value = (int)_labels.size() - 1;
    }
    else{
        _value = value;
    }
    setDirty(true);
}

const std::string MenuRadial::getActiveLabel() const
{
    return getText(_value);
}

const std::vector<std::string> MenuRadial::getLabels() const
{
    std::vector<std::string> strings;

    for(int i = 0; i < _labels.size(); ++i)
    {
        strings.push_back(getText(i));
    }

    return strings;
}

const std::string MenuRadial::getText(const int index) const
{
    if(index < 0 || index >= _labels.size())
    {
        std::cerr
                << "(MenuList) Warning: Attempting to set the value of an invalid index."
                << std::endl;
        return "";
    }
    return _labels[index];
}

void MenuRadial::setText(const int index, const std::string & text)
{
    if(index < 0 || index >= _labels.size())
    {
        std::cerr
                << "(MenuList) Warning: Attempting to set the value of an invalid index."
                << std::endl;
        return;
    }
    _labels[index]= text;
    setDirty(true);
}

void MenuRadial::setLabels(const std::vector<std::string> & l)
{
    _value = 0;
    _labels = l;
    setDirty(true);
}

void MenuRadial::addLabel(const std::string & label, const int index)
{
	setDirty(true);
	if (index < 0 || index >= _labels.size())
	{
		_labels.push_back(label);
		return;
	}
	if (_value >= index)
	{
		++_value;
	}
	_labels.insert(_labels.begin() + index, label);
}
