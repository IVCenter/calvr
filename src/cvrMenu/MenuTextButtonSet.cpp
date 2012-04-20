#include <cvrMenu/MenuTextButtonSet.h>

using namespace cvr;

MenuTextButtonSet::MenuTextButtonSet(bool radioButtons, float width,
        float rowHeight, int buttonsPerRow) :
        MenuItem()
{
    _radioButtons = radioButtons;
    _width = width;
    _rowHeight = rowHeight;
    _buttonsPerRow = buttonsPerRow;
}

MenuTextButtonSet::MenuTextButtonSet(bool radioButtons, float width,
        float rowHeight, int buttonsPerRow, std::vector<std::string> & buttons) :
        MenuItem()
{
    _radioButtons = radioButtons;
    _width = width;
    _rowHeight = rowHeight;
    _buttonsPerRow = buttonsPerRow;

    _buttons = buttons;

    for(int i = 0; i < _buttons.size(); i++)
    {
        _buttonStates.push_back(false);
    }
}

MenuTextButtonSet::~MenuTextButtonSet()
{
}

void MenuTextButtonSet::addButton(std::string button)
{
    for(int i = 0; i < _buttons.size(); i++)
    {
        if(_buttons[i] == button)
        {
            return;
        }
    }

    _buttons.push_back(button);
    _buttonStates.push_back(false);

    setDirty(true);
}

void MenuTextButtonSet::removeButton(std::string button)
{
    std::vector<std::string>::iterator it1 = _buttons.begin();
    std::vector<bool>::iterator it2 = _buttonStates.begin();
    for(; it1 != _buttons.end();)
    {
        if((*it1) == button)
        {
            it1 = _buttons.erase(it1);
            it2 = _buttonStates.erase(it2);
            setDirty(true);
        }
        else
        {
            it1++;
            it2++;
        }
    }
}

void MenuTextButtonSet::removeButton(int num)
{
    if(num < 0 || num >= _buttons.size())
    {
        return;
    }

    std::vector<std::string>::iterator it1 = _buttons.begin();
    std::vector<bool>::iterator it2 = _buttonStates.begin();

    it1 += num;
    it2 += num;

    _buttons.erase(it1);
    _buttonStates.erase(it2);
    setDirty(true);
}

void MenuTextButtonSet::clear()
{
    _buttons.clear();
    _buttonStates.clear();
    setDirty(true);
}

bool MenuTextButtonSet::getRadioButtons()
{
    return _radioButtons;
}

int MenuTextButtonSet::getButtonsPerRow()
{
    return _buttonsPerRow;
}

float MenuTextButtonSet::getWidth()
{
    return _width;
}

float MenuTextButtonSet::getRowHeight()
{
    return _rowHeight;
}

int MenuTextButtonSet::getNumButtons()
{
    return (int)_buttons.size();
}

int MenuTextButtonSet::getButtonNumber(std::string button)
{
    for(int i = 0; i < _buttons.size(); i++)
    {
        if(_buttons[i] == button)
        {
            return i;
        }
    }
    return -1;
}

const std::string & MenuTextButtonSet::getButton(int num)
{
    static std::string defaultReturn;
    if(num < 0 || num >= _buttons.size())
    {
        return defaultReturn;
    }

    return _buttons[num];
}

void MenuTextButtonSet::setValue(std::string button, bool val)
{
    int foundindex = -1;
    for(int i = 0; i < _buttons.size(); i++)
    {
        if(button == _buttons[i])
        {
            if(val == _buttonStates[i])
            {
                return;
            }

            if(val && _radioButtons)
            {
                foundindex = i;
            }

            _buttonStates[i] = val;

            setDirty(true);
            break;
        }
    }

    if(foundindex >= 0)
    {
        for(int i = 0; i < _buttons.size(); i++)
        {
            if(i != foundindex)
            {
                _buttonStates[i] = false;
            }
        }
    }
}

void MenuTextButtonSet::setValue(int num, bool val)
{
    if(num < 0 || num >= _buttons.size())
    {
        return;
    }

    if(val == _buttonStates[num])
    {
        return;
    }

    _buttonStates[num] = val;

    if(val && _radioButtons)
    {
        for(int i = 0; i < _buttons.size(); i++)
        {
            if(i != num)
            {
                _buttonStates[i] = false;
            }
        }
    }

    setDirty(true);
}

bool MenuTextButtonSet::getValue(std::string button)
{
    for(int i = 0; i < _buttons.size(); i++)
    {
        if(button == _buttons[i])
        {
            return _buttonStates[i];
        }
    }

    return false;
}

bool MenuTextButtonSet::getValue(int num)
{
    if(num < 0 || num >= _buttons.size())
    {
        return false;
    }

    return _buttonStates[num];
}

std::string MenuTextButtonSet::firstOn()
{
    for(int i = 0; i < _buttons.size(); i++)
    {
        if(_buttonStates[i])
        {
            return _buttons[i];
        }
    }
    return "";
}

int MenuTextButtonSet::firstNumOn()
{
    for(int i = 0; i < _buttons.size(); i++)
    {
        if(_buttonStates[i])
        {
            return i;
        }
    }
    return -1;
}
