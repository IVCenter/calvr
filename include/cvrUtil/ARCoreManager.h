#ifndef ARCORE_MANAGER_H
#define ARCORE_MANAGER_H

#include <cvrUtil/arcore_c_api.h>
#include <osg/Matrixf>
#include <GLES3/gl3.h>
#include <unordered_map>

namespace {
    const GLfloat kUVs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    const uint32_t kPlaneRGBA[16] = {0xFFFFFFFF, 0xF44336FF, 0xE91E63FF, 0x9C27B0FF, 0x673AB7FF, 0x3F51B5FF,
                                   0x2196F3FF, 0x03A9F4FF, 0x00BCD4FF, 0x009688FF, 0x4CAF50FF, 0x8BC34AFF,
                                   0xCDDC39FF, 0xFFEB3BFF, 0xFFC107FF, 0xFF9800FF};
    inline osg::Vec3f GetRandomPlaneColor(){
        const int32_t colorRgba = kPlaneRGBA[std::rand() % 16];
        return osg::Vec3f(((colorRgba >> 24) & 0xff) / 255.0f,
                         ((colorRgba >> 16) & 0xff) / 255.0f,
                         ((colorRgba >> 8) & 0xff) / 255.0f);
    }

}

namespace cvr{
    typedef std::unordered_map<ArPlane*, osg::Vec3f> planeMap;
    typedef  struct{float intensity = 0.8f;float color_correction[4] = {1.f, 1.f, 1.f, 1.f};} LightSrc;

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

        /**Plane Factors***/
        planeMap plane_color_map;
        std::vector<ArAnchor*> _hittedAnchors;
        /*** Lighting ***/
        LightSrc _envLight;
    public:
        static ARCoreManager * instance();
        ARCoreManager();
        ~ARCoreManager();

        void onViewChanged(int rot, int width, int height);

        void onPause();

        void onResume(void *env, void *context, void *activity);

        void onDrawFrame();

        LightSrc getLightEstimation();

        bool getPointCouldData(float*& pointCloudData, int32_t & point_num);

        bool getPlaneData(ArPlane* plane, float*& plane_data,
                          osg::Matrixf& modelMat, osg::Vec3f& normal_vec,
                          int32_t& vertice_num);

        bool updatePlaneHittest(float x, float y);

        planeMap getPlaneMap();

        size_t getAnchorSize(){return _hittedAnchors.size();}

        bool getAnchorModelMatrixAt(osg::Matrixf& modelMat, int loc);

        void setCameraTextureTarget(GLuint id){bgTextureId = id;}

        osg::Matrixf* getViewMatrix(){return view_mat;}
        osg::Matrixf* getProjMatrix(){return proj_mat;}
        osg::Matrixf  getMVPMatrix();

        const float * getCameraTransformedUVs(){return (geometry_changed)?transformed_camera_uvs:nullptr;}
        const float* getCameraPose(){return camera_pose_raw;}
    };
}



#endif