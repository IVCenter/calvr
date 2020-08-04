#ifndef WIN32
#include <sys/time.h>
#else
#include <cvrUtil/TimeOfDay.h>
#endif
#include <osg/Vec3>

class Lerp
{
    public:
        Lerp(osg::Vec3 startP, osg::Vec3 endP, float t, float d = 0,
                bool hideOnFinish = false, bool hideOnDelay = false);
        ~Lerp();
        bool isDone();
        bool isDelayed();
        bool isHideOnFinish();
        bool isHideOnDelay();
        osg::Vec3 getPosition();
        osg::Vec3 getEnd();

    protected:
        osg::Vec3 start, end;
        float seconds, delay;
        timeval startTime;
        bool hideOnFinish;
        bool hideOnDelay;
};
