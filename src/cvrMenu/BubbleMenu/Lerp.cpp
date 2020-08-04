#include <iostream>
#include <cvrMenu/BubbleMenu/Lerp.h>

Lerp::Lerp(osg::Vec3 startP, osg::Vec3 endP, float t, float d, bool hideFinish,
        bool hideDelay)
{
    start = startP;
    end = endP;
    seconds = t;
    gettimeofday(&startTime,NULL);
    delay = d;
    hideOnFinish = hideFinish;
    hideOnDelay = hideDelay;
}

Lerp::~Lerp()
{
}

bool Lerp::isDone()
{
    timeval currTime;
    gettimeofday(&currTime,NULL);

    float elapsedTime = (currTime.tv_usec + 1000000 * currTime.tv_sec)
            - (startTime.tv_usec + 1000000 * startTime.tv_sec);

    return elapsedTime / 1000000 > (seconds + delay);
}

bool Lerp::isHideOnFinish()
{
    return hideOnFinish;
}

bool Lerp::isHideOnDelay()
{
    return hideOnDelay;
}

bool Lerp::isDelayed()
{
    timeval currTime;
    gettimeofday(&currTime,NULL);

    float elapsedTime = (currTime.tv_usec + 1000000 * currTime.tv_sec)
            - (startTime.tv_usec + 1000000 * startTime.tv_sec);
    return elapsedTime < (delay * 1000000);
}

osg::Vec3 Lerp::getPosition()
{
    timeval currTime;
    gettimeofday(&currTime,NULL);

    float elapsedTime = (currTime.tv_usec + 1000000 * currTime.tv_sec)
            - (startTime.tv_usec + 1000000 * startTime.tv_sec);

    if(elapsedTime < 1000000 * delay)
    {
        return start;
    }

    float delta = (elapsedTime - 1000000 * delay) / (1000000 * seconds);

    osg::Vec3 pos = end - start;
    pos *= delta;
    pos += start;

    return pos;
}

osg::Vec3 Lerp::getEnd()
{
    return end;
}

