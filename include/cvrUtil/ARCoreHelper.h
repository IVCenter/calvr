#ifndef ARCORE_HELPER_H
#define ARCORE_HELPER_H

#include <cstdlib>
#include <cstring>
#include <glm.hpp>
#include <vector>
#include <osg/Vec3>
namespace cvr{
    class ARcoreHelper{
    public:

        static ARcoreHelper * instance();
        void getPointCloudData(const float*& pointData, int32_t& pointNum){
            pointNum = _num_of_points;
            pointData = _pointCloudData;
//            size_t mem_size =  pointNum * 4 * sizeof(float);
//            pointData = (float*)malloc(mem_size);
//            std::memcpy(pointData, _pointCloudData, mem_size);
        }
        void setPointCloudData(const float* pointData, const int32_t pointNum){
//            _num_of_points = pointNum;
//            _pointCloudData = pointData;
//            size_t mem_size =  pointNum * 4 * sizeof(float);
//            _pointCloudData = (float*)malloc(mem_size);
//            std::memcpy(_pointCloudData, pointData, mem_size);
            _randomPoints.clear();
            for(int i=0; i<std::min(pointNum, 10); i++)
                _randomPoints.push_back(glm::vec3(pointData[4*i], pointData[4*i+1], pointData[4*i+2]));
        }
        void setMVPmat(glm::mat4 mvp){ mvp_mat = mvp;}

        glm::mat4 getMVPMatrix(){return mvp_mat;}
        void setPointCloudActiveState(bool active){_isActive = active;}
        bool getActiveState(){return _isActive;}
        bool getPointCloudStatus(){return _pointcloud_on;}
        void changePointCloudStatus(){_pointcloud_on = !_pointcloud_on;}
        bool getPlaneStatus(){return _plane_on;}
        void changePlaneStatus(){_plane_on = !_plane_on;}
        void turnOnPlane(){_plane_on = true;}
        void updatePlaneData(const osg::Vec3f plane_center, int i){
            if(i>=_plane_centers.size())
                _plane_centers.push_back(plane_center);
            else
                _plane_centers[i] = plane_center;
        }
        std::vector<osg::Vec3f> getPlaneCenters(){return _plane_centers;}
        glm::vec3 getRandomPointPos(){
            return _randomPoints[std::rand() % _randomPoints.size()];
        }

    private:
        static ARcoreHelper * _myPtr;
        // Point Cloud
        const float* _pointCloudData;
        std::vector<osg::Vec3f> _plane_centers;
        int32_t _num_of_points = 0;
        bool _isActive = false;
        bool _pointcloud_on = true;
        bool _plane_on = false;
        std::vector<glm::vec3> _randomPoints;

        glm::mat4 view_mat;
        glm::mat4 proj_mat;
        glm::mat4 mvp_mat;
    };
}

#endif
