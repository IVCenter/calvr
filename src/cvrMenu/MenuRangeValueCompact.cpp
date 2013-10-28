#include <cvrMenu/MenuRangeValueCompact.h>

#include <cmath>

using namespace cvr;

MenuRangeValueCompact::MenuRangeValueCompact(std::string label, float min,
        float max, float current, bool log, float base) :
        MenuItem()
{
    _label = label;
    _min = min;
    _max = max;
    _log = log;
    _logBase = base;
    _callbackType = ON_CHANGE;

    if(_min > _max)
    {
        float t;
        t = _max;
        _max = _min;
        _min = t;
    }

    setValue(current);
}

MenuRangeValueCompact::~MenuRangeValueCompact()
{

}

MenuItemType MenuRangeValueCompact::getType()
{
    return RANGEVALUECOMPACT;
}

void MenuRangeValueCompact::setValue(float value)
{
    if(value < _min)
    {
        value = _min;
    }
    else if(value > _max)
    {
        value = _max;
    }

    _value = value;
    setDirty(true);
}

float MenuRangeValueCompact::getValue()
{
    return _value;
}

std::string MenuRangeValueCompact::getLabel()
{
    return _label;
}

float MenuRangeValueCompact::getMin()
{
    return _min;
}

float MenuRangeValueCompact::getMax()
{
    return _max;
}

bool MenuRangeValueCompact::getIsLog()
{
    return _log;
}

float MenuRangeValueCompact::getLogBase()
{
    return _logBase;
}

void MenuRangeValueCompact::setCallbackType(const CallbackType type)
{
    _callbackType = type;
}

const MenuRangeValueCompact::CallbackType MenuRangeValueCompact::getCallbackType()
{
    return _callbackType;
}
