#ifndef ANDROID_DCM_HELPER_H
#define ANDROID_DCM_HELPER_H

#include <cstdlib>
#include <GLES3/gl3.h>

class DCMI{
public:
    static size_t img_width;
    static size_t img_height;
    static size_t img_nums;
    static unsigned char* volume_data;
    static GLuint volume_tex_id;//MIAODE WOYE BUZHIDAO WEISHENME ZHEGE WORKAAAAAA

    static void assemble_texture_3d(){
        if(volume_tex_id != -1)
            return;
        glGenTextures(1, &volume_tex_id);
        // bind 3D texture target
        glBindTexture(GL_TEXTURE_3D, DCMI::volume_tex_id);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // pixel transfer happens here from client to OpenGL server
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8,
                     (int)img_width, (int)img_height, (int)img_nums,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     volume_data);
        glBindTexture(GL_TEXTURE_3D, 0);
        delete []DCMI::volume_data;
    }
};
inline GLuint DCMI::volume_tex_id = -1;
#endif