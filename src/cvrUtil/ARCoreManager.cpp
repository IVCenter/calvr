#include <cvrUtil/ARCoreManager.h>
#include <cvrUtil/AndroidStdio.h>
#include <cvrUtil/OsgGlesMath.h>


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
    ArPose_getMatrix(_ar_session, camera_pose, cameraMatrix.ptr());

    ArCamera_getTrackingState(_ar_session, camera, &cam_track_state);

    ArFrame_getDisplayGeometryChanged(_ar_session, _ar_frame, &geometry_changed);
    if (geometry_changed != 0)
        ArFrame_transformDisplayUvCoords(_ar_session, _ar_frame, 8, kUVs,
                                         transformed_camera_uvs);
    ArCamera_release(camera);
}
void ARCoreManager::postFrame(){
    if(_consumeEvent){
        _consumeEvent = false;
        _event_queue.pop();
    }
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
                                 Matrixf& modelMat, osg::Vec3f& normal_vec,
                                 int32_t& vertice_num){
    int32_t polygon_length;
    //get the number of elements(2*#vertives)
    ArPlane_getPolygonSize(_ar_session, plane, &polygon_length);

    if(polygon_length == 0)
        return false;
    vertice_num = polygon_length/2;

    plane_data = (float*)malloc(sizeof(float) * polygon_length);

    ArPlane_getPolygon(_ar_session, plane, plane_data);

    //get model matrix
    ArPose * arPose;
    ArPose_create(_ar_session, nullptr,&arPose);
    ArPlane_getCenterPose(_ar_session, plane, arPose);
    ArPose_getMatrix(_ar_session, arPose, modelMat.ptr());

    //get normal vector
    float plane_pose_raw[7] = {.0f};
    ArPose_getPoseRaw(_ar_session, arPose, plane_pose_raw);
    osg::Quat plane_quaternion(plane_pose_raw[0],plane_pose_raw[1], plane_pose_raw[2],plane_pose_raw[3]);
    // Get normal vector, normal is defined to be positive Y-position in local
    // frame.
    normal_vec = plane_quaternion * osg::Vec3f(0,1.0f,0);
    return true;
}
LightSrc ARCoreManager::getLightEstimation(){
    ArLightEstimate* ar_light_estimate;
    ArLightEstimateState ar_light_estimate_state;
    ArLightEstimate_create(_ar_session, &ar_light_estimate);

    ArFrame_getLightEstimate(_ar_session, _ar_frame, ar_light_estimate);
    ArLightEstimate_getState(_ar_session, ar_light_estimate, &ar_light_estimate_state);
    if(ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID){
        ArLightEstimate_getColorCorrection(_ar_session, ar_light_estimate,  _envLight.color_correction);
        ArLightEstimate_getPixelIntensity(_ar_session, ar_light_estimate, &_envLight.intensity);
    }

    ArLightEstimate_destroy(ar_light_estimate);
    return _envLight;
}

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

bool ARCoreManager::updatePlaneHittest(float x, float y){
    _event_queue.push(Vec2f(x,y));
    if(!_ar_frame || !_ar_session) return false;
    ArHitResultList* hit_result_list = nullptr;
    ArHitResultList_create(_ar_session, &hit_result_list);
    CHECK(hit_result_list);
    ArFrame_hitTest(_ar_session, _ar_frame, x, y, hit_result_list);

    int32_t hit_result_list_size = 0;
    ArHitResultList_getSize(_ar_session, hit_result_list,
                            &hit_result_list_size);

    ArHitResult* ar_hit_result = nullptr;

    for (int32_t i = 0; i < hit_result_list_size; ++i) {
        ArHitResult *ar_hit = nullptr;
        ArHitResult_create(_ar_session, &ar_hit);
        ArHitResultList_getItem(_ar_session, hit_result_list, i, ar_hit);

        if (ar_hit == nullptr) {
            LOGE("HelloArApplication::OnTouched ArHitResultList_getItem error");
            return false;
        }

        ArTrackable *ar_trackable = nullptr;
        ArHitResult_acquireTrackable(_ar_session, ar_hit, &ar_trackable);
        ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
        ArTrackable_getType(_ar_session, ar_trackable, &ar_trackable_type);
        ///////////////ONLY CHECK PLANE/////////////////////////////
        if(ar_trackable_type != AR_TRACKABLE_PLANE)
            continue;

        ArPose* hit_pose = nullptr;
        ArPose_create(_ar_session, nullptr, &hit_pose);
        ArHitResult_getHitPose(_ar_session, ar_hit, hit_pose);
        int32_t in_polygon = 0;
        ArPlane* ar_plane = ArAsPlane(ar_trackable);
        ArPlane_isPoseInPolygon(_ar_session, ar_plane, hit_pose, &in_polygon);

        float plane_pose_raw[7] = {0.f};
        ArPose_getPoseRaw(_ar_session, hit_pose, plane_pose_raw);

        ArPose_destroy(hit_pose);

        if (!in_polygon || calculateDistanceToPlane(plane_pose_raw, camera_pose_raw) < 0)
            continue;

        ar_hit_result = ar_hit;
        break;
    }
    ////////////////////////////
    if(ar_hit_result){
        ArAnchor* anchor = nullptr;
        if (ArHitResult_acquireNewAnchor(_ar_session, ar_hit_result, &anchor) !=
            AR_SUCCESS) {
            LOGE(
                    "HelloArApplication::OnTouched ArHitResult_acquireNewAnchor error");
            return false;
        }

        ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
        ArAnchor_getTrackingState(_ar_session, anchor, &tracking_state);
        if (tracking_state != AR_TRACKING_STATE_TRACKING) {
            ArAnchor_release(anchor);
            return false;
        }

        _hittedAnchors.push_back(anchor);
        ArHitResult_destroy(ar_hit_result);
        ar_hit_result = nullptr;

        ArHitResultList_destroy(hit_result_list);
        hit_result_list = nullptr;
    }
    return true;
}

bool ARCoreManager::getAnchorModelMatrixAt(Matrixf& modelMat, int loc, bool realCoord){
    if(loc>=_hittedAnchors.size())
        return false;
    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
    ArAnchor_getTrackingState(_ar_session, _hittedAnchors[loc],
                              &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
        ArPose* pose_;
        ArPose_create(_ar_session, nullptr, &pose_);
        ArAnchor_getPose(_ar_session, _hittedAnchors[loc], pose_);

        if(!realCoord){
            float pose_raw[7];
            ArPose_getPoseRaw(_ar_session, pose_, pose_raw);
            modelMat = cvr::rawRotation2OsgMatrix(pose_raw)* rawTrans2OsgMatrix(pose_raw+4);
        }else
            ArPose_getMatrix(_ar_session, pose_, modelMat.ptr());

        ArPose_destroy(pose_);
    }
    return true;
}

osg::Matrixf ARCoreManager::getMVPMatrix(){Matrixf mat = (*view_mat) * (*  proj_mat);
    return mat;}
osg::Vec3f ARCoreManager::getRealWorldPositionFromScreen(float x, float y, float z){
    if(x > 1 || x < -1 || y>1 || y<-1){LOGE("Position should within [-1, 1]"); return osg::Vec3f(.0,.0,.0);}
    Matrixf invMat = Matrixf::inverse(getMVPMatrix());
    Vec4f nearPlanePos = Vec4f(x,y, z, 1.0f) * invMat;
    float inv_w = 1.0f / nearPlanePos.w();
    return Vec3f(nearPlanePos.x() * inv_w, nearPlanePos.y()*inv_w, nearPlanePos.z()*inv_w);
}