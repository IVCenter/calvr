/**
 * @file PointsNode.h
 */
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

/**
 * @brief OpenSceneGraph Node that acts as a wrapper for display and manipulation
 *        of a collection of points
 *
 * Points are set with a radius, size and color value.  The user may set and change the
 * point rendering mode.  For modes that use a physical size, the radius value is used.
 * Otherwise, the point size is used.
 */
class PointsNode : public osg::Group
{
    friend class PointsUpdateCallback;
    public:
        /**
         * @brief Used to describe the binding for a point attribute
         */
        enum PointsBinding
        {
            POINTS_PER_POINT = 0,
            POINTS_OVERALL
        };

        /**
         * @brief Different modes that can be used to render the set of points
         */
        enum PointsMode
        {
            POINTS_GL_POINTS = 0,
            POINTS_POINT_SPRITES,
            POINTS_SHADED_SPHERES
        };

        /**
         * @brief Constructor
         * @param mode Initial point rendering mode
         * @param startingNumPoints Number of points to pre-allocate space for
         * @param defaultPointSize Point size to use if none is give
         * @param defaultRadius Radius to use if none is given
         * @param defaultColor Color to use if none is given
         * @param sizeBinding If size should be per-point or overall
         * @param radiusBinding If radius should be per-point or overall
         * @param colorBinding If color should be per-point or overall
         */
        PointsNode(PointsMode mode, int startingNumPoints, float defaultPointSize, float defaultRadius, osg::Vec4ub defaultColor, PointsBinding sizeBinding = POINTS_PER_POINT, PointsBinding radiusBinding = POINTS_PER_POINT, PointsBinding colorBinding = POINTS_PER_POINT);

        /**
         * @brief Constructor
         * @param mode Initial point rendering mode
         * @param startingNumPoints Number of points to pre-allocate space for
         * @param defaultPointSize Point size to use if none is give
         * @param defaultRadius Radius to use if none is given
         * @param defaultColor Color to use if none is given
         * @param sizeBinding If size should be per-point or overall
         * @param radiusBinding If radius should be per-point or overall
         * @param colorBinding If color should be per-point or overall
         */
        PointsNode(PointsMode mode, int startingNumPoints, float defaultPointSize, float defaultRadius, osg::Vec4 defaultColor, PointsBinding sizeBinding = POINTS_PER_POINT, PointsBinding radiusBinding = POINTS_PER_POINT, PointsBinding colorBinding = POINTS_PER_POINT);
        PointsNode(const PointsNode & pn, const osg::CopyOp & copyop=osg::CopyOp::SHALLOW_COPY);

        /**
         * @brief Set the vertex array for the set of points
         *
         * A NULL array has no action
         */
        void setVertexArray(osg::Vec3Array * vertArray);

        /**
         * @brief Set the color array for the set of points
         *
         * A NULL array has no action
         */
        void setColorArray(osg::Vec4ubArray * colorArray);

        /**
         * @brief Set the color for all points
         *
         * Note: this becomes the new default color value
         */
        void setColor(osg::Vec4ub color);

        /**
         * @brief Set the color for all points
         *
         * Note: this becomes the new default color value
         */
        void setColor(osg::Vec4 color);

        /**
         * @brief Set the point size array for the set of points
         *
         * A NULL array has no action
         */
        void setPointSizeArray(osg::FloatArray * sizeArray);

        /**
         * @brief Set the point size for all points
         *
         * Note: this becomes the new default point size
         */
        void setPointSize(float size);

        /**
         * @brief Set the radius array for the set of points
         *
         * A NULL array has no action
         */
        void setRadiusArray(osg::FloatArray * radiusArray);

        /**
         * @brief Set the radius for all points
         *
         * Note: this becomes the new default radius
         */
        void setRadius(float radius);

        /**
         * @brief Get the number of valid points
         *
         * This value is the size of the smallest array with per-point attributes
         */
        int getNumPoints()
        {
            return _size;
        }

        /**
         * @brief Set the values for a given point
         * @param pointIndex Point number to set
         * @param position New point position
         * @param color New point color
         * @param radius New point radius
         * @param size New point size
         *
         * If the pointIndex is 0, attributes with an POINTS_OVERALL binding are set
         */
        void setPoint(int pointIndex, osg::Vec3 position, osg::Vec4ub color, float radius, float size);

        /**
         * @brief Set the values for a given point
         * @param pointIndex Point number to set
         * @param position New point position
         * @param color New point color
         * @param radius New point radius
         * @param size New point size
         *
         * If the pointIndex is 0, attributes with an POINTS_OVERALL binding are set
         */
        void setPoint(int pointIndex, osg::Vec3 position, osg::Vec4 color, float radius, float size);

        /**
         * @brief Set the position of a given point
         */
        void setPointPosition(int pointIndex, osg::Vec3 position);

        /**
         * @brief Set the color of a given point
         *
         * If the pointIndex is 0 and the color binding is POINTS_OVERALL, the overall color is set
         */
        void setPointColor(int pointIndex, osg::Vec4ub color);

        /**
         * @brief Set the color of a given point
         *
         * If the pointIndex is 0 and the color binding is POINTS_OVERALL, the overall color is set
         */
        void setPointColor(int pointIndex, osg::Vec4 color);

        /**
         * @brief Set the radius of a given point
         *
         * If the pointIndex is 0 and the radius binding is POINTS_OVERALL, the overall radius is set
         */
        void setPointRadius(int pointIndex, float radius);

        /**
         * @brief Set the point size of a given point
         *
         * If the pointIndex is 0 and the point size binding is POINTS_OVERALL, the overall size is set
         */
        void setPointSize(int pointIndex, float size);
        
        /**
         * @brief Get the position of a given point
         */
        osg::Vec3 getPointPosition(int pointIndex);

        /**
         * @brief Get the color of a given point as a Vec4ub
         */
        osg::Vec4ub getPointColor(int pointIndex);

        /**
         * @brief Get the color of a given point as a Vec4
         */
        osg::Vec4 getPointColorF(int pointIndex);

        /**
         * @brief Get the radius of a given point
         */
        float getPointRadius(int pointIndex);

        /**
         * @brief Get the point size of a given point
         */
        float getPointSize(int pointIndex);

        /**
         * @brief Add a point to the set with the given position
         *
         * The default values will be used for all point attributes
         */
        void addPoint(osg::Vec3 position);

        /**
         * @brief Add a point to the set with the given attributes
         *
         * If the radius or size values are less than 0, the default values will be used
         */
        void addPoint(osg::Vec3 position, osg::Vec4ub color, float radius = -1, float size = -1);

        /**
         * @brief Add a point to the set with the given attributes
         *
         * If the radius or size values are less than 0, the default values will be used
         */
        void addPoint(osg::Vec3 position, osg::Vec4 color, float radius = -1, float size = -1);

        /**
         * @brief Remove the point at the given index from the set
         */
        void removePoint(int pointIndex);

        /**
         * @brief Remove points from the set
         * @param startPoint First point to be removed
         * @param numPoints Number of points to remove
         */
        void removePoints(int startPoint, int numPoints);

        /**
         * @brief Clear all points and attributes from this set
         */
        void clear();

        /**
         * @brief Set the texture to use when rendering point sprites
         */
        void setSpriteTexture(osg::Texture2D * texture);

        /**
         * @brief Get the texture used for rendering point sprites
         */
        osg::Texture2D * getSpriteTexture()
        {
            return _spriteTexture.get();
        }

        /**
         * @brief Get the OpenSceneGraph Point class
         *
         * This is only used during POINTS_POINT_SPRITES rendering mode
         */
        osg::Point * getOsgPoint()
        {
            return _point.get();
        }

        /**
         * @brief Get the OpenSceneGraph PointSprite class
         *
         * This is only used during POINTS_POINT_SPRITES rendering mode
         */
        osg::PointSprite * getOsgPointSprite()
        {
            return _pointSprite.get();
        }

        /**
         * @brief Get the OpenSceneGraph BlendFunc class
         *
         * This is only used during POINTS_POINT_SPRITES rendering mode
         */
        osg::BlendFunc * getOsgPointSpriteBlendFunc()
        {
            return _blendFunc.get();
        }

        /**
         * @brief Set the point rendering mode
         */
        void setPointsMode(PointsMode mode);

    protected:
        virtual ~PointsNode();

        /**
         * @brief Init for the class, called by the constructors
         */
        void init(PointsMode mode, int startingNumPoints, float defaultPointSize, float defaultRadius, osg::Vec4ub & defaultColor, PointsBinding sizeBinding, PointsBinding radiusBinding, PointsBinding colorBinding);

        /**
         * @brief Called during the update traversal to update shader uniforms
         */
        void update();

        /**
         * @brief Calculate the size of the point set using the array sizes
         */
        void calcSize();

        /**
         * @brief Refresh the arrays into the geometry
         */
        void refreshGeometry();

        /**
         * @brief Node callback used to catch the update traveral
         */
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

        //void makeTexture();

        osg::ref_ptr<PointsUpdateCallback> _updateCallback; ///< callback to catch update traveral

        int _size; ///< current number of valid points in the set
        PointsMode _mode; ///< current point rendering mode
        PointsBinding _colorBinding; ///< binding for color attribute
        PointsBinding _radiusBinding; ///< binding for radius attribute
        PointsBinding _sizeBinding; ///< binding for point size attribute

        osg::ref_ptr<osg::Vec3Array> _vertArray; ///< array of point vertex data
        osg::ref_ptr<osg::Vec4ubArray> _colorArray; ///< array of point color data
        osg::ref_ptr<osg::FloatArray> _sizeArray; ///< array of point size data
        osg::ref_ptr<osg::FloatArray> _radiusArray; ///< array of point radius data
        osg::ref_ptr<osg::Point> _point; ///< OpenSceneGraph point attriubtes
        osg::ref_ptr<osg::BlendFunc> _blendFunc; ///< OpenSceneGraph blend function
        osg::ref_ptr<osg::PointSprite> _pointSprite; ///< OpenSceneGraph point sprite attributes

        float _pointSize; ///< default point size
        float _pointRadius; ///< default point radius
        osg::Vec4ub _pointColor; ///< default point color

        osg::ref_ptr<osg::Program> _programPoints; ///< shader program for GL_POINTS
        osg::ref_ptr<osg::Program> _programSphere; ///< shader program for shaded spheres
        osg::ref_ptr<osg::Uniform> _scaleUni; ///< scale uniform for shaded sphere shader

        osg::ref_ptr<osg::Geometry> _geometry; ///< geometry for points
        osg::ref_ptr<osg::Geode> _geode; ///< geode for points

        osg::ref_ptr<osg::DrawArrays> _primitive; ///< points primitive

        osg::ref_ptr<osg::Texture2D> _spriteTexture; ///< texture for point sprites
        //static osg::ref_ptr<osg::Texture2D> _sphereTexture;
};

}
#endif
