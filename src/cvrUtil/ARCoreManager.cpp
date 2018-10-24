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
bool ARCoreManager::getPlaneData(ArPlane* plane, float*& plane_data,
                                 Matrixf& modelMat, int32_t& vertice_num){
    int32_t polygon_length;
    //get the number of elements(2*#vertives)
    ArPlane_getPolygonSize(_ar_session, plane, &polygon_length);

    if(polygon_length == 0){
        LOGE("NO valid plane polygon found");
        return false;
    }
    vertice_num = polygon_length/2;

    plane_data = (float*)malloc(sizeof(float) * polygon_length);

    ArPlane_getPolygon(_ar_session, plane, plane_data);

    //get model matrix
    ArPose * arPose;
    ArPose_create(_ar_session, nullptr,&arPose);
    ArPlane_getCenterPose(_ar_session, plane, arPose);
    ArPose_getMatrix(_ar_session, arPose, modelMat.ptr());

    return true;
}
//void ARCoreManager::doLightEstimation(){
//    ArLightEstimate* ar_light_estimate;
//    ArLightEstimateState ar_light_estimate_state;
//    ArLightEstimate_create(_ar_session, &ar_light_estimate);
//
//    ArFrame_getLightEstimate(_ar_session, _ar_frame, ar_light_estimate);
//    ArLightEstimate_getState(_ar_session, ar_light_estimate, &ar_light_estimate_state);
//    if(ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID){
//        ArLightEstimate_getColorCorrection(_ar_session, ar_light_estimate, _light.color_correction);
//        ArLightEstimate_getPixelIntensity(_ar_session, ar_light_estimate, &_light.intensity);
//    }
//
//    ArLightEstimate_destroy(ar_light_estimate);
//}

planeMap ARCoreManager::getPlaneMap(){
    int detectedPlaneNum;
    ArTrackableList* plane_list = nullptr;
    ArTrackableList_create(_ar_session, & plane_list);
    CHECK(plane_list!= nullptr);

    ArSession_getAllTrackables(_ar_session, AR_TRACKABLE_PLANE, plane_list);
    ArTrackableList_getSize(_ar_session, plane_list, &detectedPlaneNum);

    for(int i=0; i<detectedPlaneNum; i++){
        ArTrackable * ar_trackable = nullptr;
        ArTrackableList_acquireItem(_ar_session, plane_list, i, &ar_trackable);

        //cast down trackable to plane
        ArPlane * ar_plane = ArAsPlane(ar_trackable);

        //check the trackingstate, if not tracking, skip rendering
        ArTrackingState trackingState;
        ArTrackable_getTrackingState(_ar_session, ar_trackable, &trackingState);
        if(trackingState != AR_TRACKING_STATE_TRACKING)
            continue;

        //check if the plane contain the subsume plane, if so, skip to avoid overlapping
        ArPlane * subsume_plane;
        ArPlane_acquireSubsumedBy(_ar_session, ar_plane, &subsume_plane);
        if(subsume_plane != nullptr)
            continue;

        auto iter = plane_color_map.find(ar_plane);
        if(iter == plane_color_map.end()){
            if(plane_color_map.empty())
                plane_color_map[ar_plane] = osg::Vec3f(1.0f, 1.0f, 1.0f);
            else
                plane_color_map[ar_plane] = GetRandomPlaneColor();
        }
        ArTrackable_release(ar_trackable);
    }
    ArTrackableList_destroy(plane_list);
    plane_list = nullptr;
    return plane_color_map;
}
osg::Matrixf ARCoreManager::getMVPMatrix(){Matrixf mat = (*view_mat) * (*  proj_mat);
    return mat;}