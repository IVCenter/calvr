#include <cvrUtil/ARCoreManager.h>
#include <cvrUtil/AndroidStdio.h>


using namespace cvr;
using namespace osg;
ARCoreManager::ARCoreManager(){
    view_mat = new Matrixf;
    proj_mat = new Matrixf;
}

ARCoreManager* ARCoreManager::_myPtr = nullptr;
ARCoreManager *ARCoreManager::instance() {
    if(!_myPtr) _myPtr = new ARCoreManager;
    return _myPtr;
}

ARCoreManager::~ARCoreManager() {
    if(_ar_session){
        ArSession_destroy(_ar_session);
        ArFrame_destroy(_ar_frame);
    }
}

void ARCoreManager::onViewChanged(int rot, int width, int height){
    _displayRotation = rot;
    _width = width;
    _height = height;
    if (_ar_session != nullptr)
        ArSession_setDisplayGeometry(_ar_session, rot, width, height);
}

void ARCoreManager::onPause(){
    if(_ar_session) ArSession_pause(_ar_session);
}

void ARCoreManager::onResume(void *env, void *context, void *activity){
    if(nullptr == _ar_session){
        ArInstallStatus install_status;
        bool user_requested_install = !_install_requested;

//            CHECK(ArCoreApk_requestInstall(env, activity, user_requested_install,&install_status) == AR_SUCCESS);

        switch (install_status) {
            case AR_INSTALL_STATUS_INSTALLED:
                break;
            case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                _install_requested = true;
                return;
        }
        CHECK(ArSession_create(env, context, &_ar_session)==AR_SUCCESS);
        CHECK(_ar_session);
        ArFrame_create(_ar_session, &_ar_frame);
        CHECK(_ar_frame);
        ArSession_setDisplayGeometry(_ar_session, _displayRotation, _width, _height);
    }
    const ArStatus status = ArSession_resume(_ar_session);
    CHECK(status == AR_SUCCESS);
}

void ARCoreManager::onDrawFrame() {
    if(_ar_session == nullptr)
        return;
    ArSession_setCameraTextureName(_ar_session, bgTextureId);
    // Update session to get current frame and render camera background.
    if (ArSession_update(_ar_session, _ar_frame) != AR_SUCCESS) {
        LOGE("OnDrawFrame ArSession_update error");
    }
    ArCamera* camera;
    ArFrame_acquireCamera(_ar_session, _ar_frame, &camera);

    ArCamera_getViewMatrix(_ar_session, camera, (*view_mat).ptr());
    ArCamera_getProjectionMatrix(_ar_session,camera, 0.1f, 100.0f, (*proj_mat).ptr());

    ArPose* camera_pose = nullptr;
    ArPose_create(_ar_session, nullptr, &camera_pose);
    ArCamera_getPose(_ar_session, camera, camera_pose);

    ArPose_getPoseRaw(_ar_session, camera_pose, camera_pose_raw);

    ArCamera_getTrackingState(_ar_session, camera, &cam_track_state);

    ArFrame_getDisplayGeometryChanged(_ar_session, _ar_frame, &geometry_changed);
    if (geometry_changed != 0)
        ArFrame_transformDisplayUvCoords(_ar_session, _ar_frame, 8, kUVs,
                                         transformed_camera_uvs);
    ArCamera_release(camera);
}

bool ARCoreManager::getPointCouldData(float*& pointCloudData, int32_t & point_num){
    ArPointCloud * pointCloud;
    ArStatus  pointcloud_Status = ArFrame_acquirePointCloud(_ar_session, _ar_frame, &pointCloud);
    if(pointcloud_Status != AR_SUCCESS)
        return false;

    ArPointCloud_getNumberOfPoints(_ar_session, pointCloud, &point_num);
    if(point_num <= 0)
        return false;
    const float* _pointCloudData;
    //point cloud data with 4 params (x,y,z, confidence)
    ArPointCloud_getData(_ar_session, pointCloud, &_pointCloudData);

    ArPointCloud_release(pointCloud);
    size_t memsize = 4 * sizeof(float) * point_num;
    pointCloudData = (float*)malloc(memsize);
    memcpy(pointCloudData, _pointCloudData, memsize);
    return true;
}
osg::Matrixf ARCoreManager::getMVPMatrix(){Matrixf mat = (*view_mat) * (*  proj_mat);
    return mat;}