#include <cvrUtil/ARCoreHelper.h>
using namespace cvr;
ARcoreHelper *ARcoreHelper:: _myPtr = nullptr;
ARcoreHelper * ARcoreHelper::instance() {
    if(!_myPtr) _myPtr = new ARcoreHelper;
    return _myPtr;
}