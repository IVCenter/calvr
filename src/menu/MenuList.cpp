#include <menu/MenuList.h>
#include <iostream>

using namespace cvr;

MenuList::MenuList() :
        MenuItem()
{
    _index = 0;
    _focusMargin = 2;
    _sensitivity = -1;
}

MenuList::~MenuList()
{
}

MenuItemType MenuList::getType()
{
    return LIST;
}

void MenuList::setValue(const int index, const std::string & value)
{
    if(index < 0 || index >= _values.size())
    {
        std::cerr
                << "(MenuList) Warning: Attempting to set the value of an invalid index."
                << std::endl;
        return;
    }

    _values[index] = value;
    setDirty(true);
}

const std::string MenuList::getValue() const
{
    return getValue((int)_index);
}

const std::string MenuList::getValue(int index) const
{
    if(index < 0 || index >= _values.size() || _values.empty())
        return "";
    return _values[index];
}

void MenuList::setValues(const std::vector<std::string> & v)
{
    _index = 0;
    _values = v;
    setDirty(true);
}

const std::vector<std::string> MenuList::getValues() const
{
    std::vector<std::string> strings;

    const unsigned int count = _focusMargin * 2 + 1;

    for(int i = 0; i < count; i++)
    {
        strings.push_back(getValue((int)_index + i - _focusMargin));
    }

    return strings;
}

const int MenuList::getIndex() const
{
    return _index;
}

void MenuList::setIndex(const int index)
{
    if(index < 0)
        _index = 0;
    else if(index >= _values.size())
        _index = (int)_values.size() - 1;
    else
        _index = index;

    setDirty(true);
}

const unsigned int MenuList::getFocus() const
{
    return _focusMargin;
}

void MenuList::matchIndexToValue(const std::string & str)
{
    for(unsigned int i = 0; i < _values.size(); i++)
        if(_values[i] == str)
        {
            setIndex(i);
            return;
        }
}

void MenuList::setFocus(const unsigned int size)
{
    _focusMargin = size;
    setDirty(true);
}

const int MenuList::getListSize() const
{
    return (int)_values.size();
}

void MenuList::setSensitivity(const float sensitivity)
{
    _sensitivity = sensitivity;
}

const float MenuList::getSensitivity()
{
    if(_sensitivity < 0)
        return (float)getListSize();
    else
        return _sensitivity;
}
