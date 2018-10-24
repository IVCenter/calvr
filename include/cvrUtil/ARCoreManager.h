#ifndef ARCORE_MANAGER_H
#define ARCORE_MANAGER_H

#include <cvrUtil/arcore_c_api.h>
#include <osg/Matrixf>
#include <GLES3/gl3.h>

namespace {
    const GLfloat kUVs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
}

namespace cvr{
    class ARCoreManager{
    private:
        static ARCoreManager* _myPtr;
        ArSession * _ar_session = nullptr;
        ArFrame * _ar_frame = nullptr;//get frame state

        int _displayRotation = 0;
        int _width = 1;
        int _height = 1;
        bool _install_requested = false;

        /* Camera factors: pos, VP matrix, tracking state
         * Update every frame
         * **/
        osg::Matrixf *view_mat, *proj_mat;
        ArTrackingState cam_track_state;
        float camera_pose_raw[7] = {0.f};
        float transformed_camera_uvs[8] = {.0f};
        GLuint bgTextureId = 0;
        int32_t geometry_changed = 0;

    public:
        static ARCoreManager * instance();
        ARCoreManager();
        ~ARCoreManager();

        void onViewChanged(int rot, int width, int height);

        void onPause();

        void onResume(void *env, void *context, void *activity);

        void onDrawFrame();

        bool getPointCouldData(float*& pointCloudData, int32_t & point_num);

        void setCameraTextureTarget(GLuint id){bgTextureId = id;}

        osg::Matrixf* getViewMatrix(){return view_mat;}
        osg::Matrixf* getProjMatrix(){return proj_mat;}
        osg::Matrixf  getMVPMatrix();

        const float * getCameraTransformedUVs(){return (geometry_changed)?transformed_camera_uvs:nullptr;}
        const float* getCameraPose(){return camera_pose_raw;}
    };
}



#endif
