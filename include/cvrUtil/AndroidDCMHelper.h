#ifndef ANDROID_DCM_HELPER_H
#define ANDROID_DCM_HELPER_H

#include <cstdlib>
#include <GLES3/gl3.h>

class DCMI{
public:
    static size_t img_width;
    static size_t img_height;
    static size_t img_nums;
    static GLubyte* volume_data;
};

#endif