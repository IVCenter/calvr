#include <cvrMenu/MenuRangeValue.h>

using namespace cvr;

MenuRangeValue::MenuRangeValue(std::string label, float min, float max,
        float current, float stepsize) :
        MenuItem()
{
    _label = label;
    _min = min;
    _max = max;

    if(_min > _max)
    {
        float t;
        t = _max;
        _max = _min;
        _min = t;
    }

    setValue(current);
    if(stepsize == 0.0)
    {
        _stepSize = (_max - _min) * 0.05;
    }
    else if(stepsize < 0.0)
    {
        _stepSize = fabs(stepsize);
    }
    else
    {
        _stepSize = stepsize;
    }
}

MenuRangeValue::~MenuRangeValue()
{

}

MenuItemType MenuRangeValue::getType()
{
    return RANGEVALUE;
}

void MenuRangeValue::setValue(float value)
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

float MenuRangeValue::getValue()
{
    return _value;
}

std::string MenuRangeValue::getLabel()
{
    return _label;
}

float MenuRangeValue::getMin()
{
    return _min;
}

float MenuRangeValue::getMax()
{
    return _max;
}
