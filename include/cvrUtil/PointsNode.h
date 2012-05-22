#ifndef CALVR_POINTS_NODE_H
#define CALVR_POINTS_NODE_H

#include <osg/Group>
#include <osg/Array>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Point>
#include <osg/PointSprite>
#include <osg/BlendFunc>
#include <osg/PrimitiveSet>
#include <osg/Texture2D>
#include <OpenThreads/Mutex>

namespace cvr
{

class PointsUpdateCallback;

class PointsNode : public osg::Group
{
    friend class PointsUpdateCallback;
    public:
        enum PointsBinding
        {
            POINTS_PER_POINT = 0,
            POINTS_OVERALL
        };

        enum PointsMode
        {
            POINTS_GL_POINTS = 0,
            POINTS_POINT_SPRITES,
            POINTS_SHADED_SPHERES
        };

        PointsNode(PointsMode mode = POINTS_SHADED_SPHERES, int startingNumPoints = 0, float defaultPointSize = 1.0, float defaultRadius = 50.0, osg::Vec4ub defaultColor = osg::Vec4ub(255,255,255,255), PointsBinding sizeBinding = POINTS_PER_POINT, PointsBinding radiusBinding = POINTS_PER_POINT, PointsBinding colorBinding = POINTS_PER_POINT);
        PointsNode(const PointsNode & pn, const osg::CopyOp & copyop=osg::CopyOp::SHALLOW_COPY);

        void setVertexArray(osg::Vec3Array * vertArray);
        void setColorArray(osg::Vec4ubArray * colorArray);
        void setColor(osg::Vec4ub color);
        void setColor(osg::Vec4 color);
        void setPointSizeArray(osg::FloatArray * sizeArray);
        void setPointSize(float size);
        void setRadiusArray(osg::FloatArray * radiusArray);
        void setRadius(float radius);

        float getNumPoints()
        {
            return _size;
        }

        void setPoint(int pointIndex, osg::Vec3 position, osg::Vec4ub color, float radius, float size);
        void setPoint(int pointIndex, osg::Vec3 position, osg::Vec4 color, float radius, float size);
        void setPointPosition(int pointIndex, osg::Vec3 position);
        void setPointColor(int pointIndex, osg::Vec4ub color);
        void setPointColor(int pointIndex, osg::Vec4 color);
        void setPointRadius(int pointIndex, float radius);
        void setPointSize(int pointIndex, float size);
        
        osg::Vec3 getPointPosition(int pointIndex);
        osg::Vec4ub getPointColor(int pointIndex);
        osg::Vec4 getPointColorF(int pointIndex);
        float getPointRadius(int pointIndex);
        float getPointSize(int pointIndex);

        void addPoint(osg::Vec3 position);
        void addPoint(osg::Vec3 position, osg::Vec4ub color, float radius = -1, float size = -1);
        void addPoint(osg::Vec3 position, osg::Vec4 color, float radius = -1, float size = -1);

        osg::Point * getOsgPoint()
        {
            return _point.get();
        }
        osg::PointSprite * getOsgPointSprite()
        {
            return _pointSprite.get();
        }
        osg::BlendFunc * getOsgPointSpriteBlendFunc()
        {
            return _blendFunc.get();
        }

        void setPointsMode(PointsMode mode);

    protected:
        virtual ~PointsNode();

        void update();
        void calcSize();
        void refreshGeometry();

        //void makeTexture();

        osg::ref_ptr<PointsUpdateCallback> _updateCallback;

        int _size;
        PointsMode _mode;
        PointsBinding _colorBinding;
        PointsBinding _radiusBinding;
        PointsBinding _sizeBinding;

        osg::ref_ptr<osg::Vec3Array> _vertArray;
        osg::ref_ptr<osg::Vec4ubArray> _colorArray;
        osg::ref_ptr<osg::FloatArray> _sizeArray;
        osg::ref_ptr<osg::FloatArray> _radiusArray;
        osg::ref_ptr<osg::Point> _point;
        osg::ref_ptr<osg::BlendFunc> _blendFunc;
        osg::ref_ptr<osg::PointSprite> _pointSprite;

        float _pointSize;
        float _pointRadius;
        osg::Vec4ub _pointColor;

        osg::ref_ptr<osg::Program> _programPoints;
        osg::ref_ptr<osg::Program> _programSphere;
        osg::ref_ptr<osg::Uniform> _scaleUni;

        osg::ref_ptr<osg::Geometry> _geometry;
        osg::ref_ptr<osg::Geode> _geode;

        osg::ref_ptr<osg::DrawArrays> _primitive;

        osg::ref_ptr<osg::Texture2D> _spriteTexture;
        //static osg::ref_ptr<osg::Texture2D> _sphereTexture;
};

class PointsUpdateCallback : public osg::NodeCallback
{
    public:
        PointsUpdateCallback(PointsNode * pn)
        {
            _pointsNode = pn;
        }
        virtual void operator()(osg::Node *node, osg::NodeVisitor *nv)
        {
            _pointsNode->update();
        }
    protected:
        PointsNode * _pointsNode;
};

}
#endif
