#ifndef ARCORE_HELPER_H
#define ARCORE_HELPER_H

#include <cstdlib>
#include <cstring>
#include <glm.hpp>

namespace cvr{
    class ARcoreHelper{
    public:
        glm::mat4 view_mat;
        glm::mat4 proj_mat;
        static ARcoreHelper * instance();
        void getPointCloudData(const float*& pointData, int32_t& pointNum){
            pointNum = _num_of_points;
            pointData = _pointCloudData;
//            size_t mem_size =  pointNum * 4 * sizeof(float);
//            pointData = (float*)malloc(mem_size);
//            std::memcpy(pointData, _pointCloudData, mem_size);
        }
        void setPointCloudData(const float* pointData, int32_t pointNum){
            _num_of_points = pointNum;
            _pointCloudData = pointData;
//            size_t mem_size =  pointNum * 4 * sizeof(float);
//            _pointCloudData = (float*)malloc(mem_size);
//            std::memcpy(_pointCloudData, pointData, mem_size);
        }
        glm::mat4 getMVPMatrix(){return proj_mat * view_mat;}
        void setPointCloudActiveState(bool active){_isActive = active;}
        bool getActiveState(){return _isActive;}
        bool getPointCloudStatus(){return _pointcloud_on;}
        void changePointCloudStatus(){_pointcloud_on = !_pointcloud_on;}
        bool getPlaneStatus(){return _plane_on;}
        void changePlaneStatus(){_plane_on = !_plane_on;}

    private:
        static ARcoreHelper * _myPtr;
        // Point Cloud
        const float* _pointCloudData;
        int32_t _num_of_points = 0;
        bool _isActive = false;
        bool _pointcloud_on = false;
        bool _plane_on = false;
    };
}

#endif
