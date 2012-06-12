/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
*
* This library is open source and may be redistributed and/or modified under
* the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
* (at your option) any later version.  The full license is in LICENSE file
* included with this distribution, and on the openscenegraph.org website.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* OpenSceneGraph Public License for more details.
*/
// Modified version of StatsHandler used by OSG
/**
 * @file CVRStatsHandler.h
 */

#ifndef CVR_STATS_HANDLER_H
#define CVR_STATS_HANDLER_H

#include <cvrKernel/Export.h>

#include <osgGA/GUIEventHandler>
#include <osgViewer/ViewerBase>
#include <osgText/Text>
#include <osg/Camera>
#include <osg/Geometry>
#include <osg/Switch>
#include <osg/Stats>
#include <osg/Vec3>
#include <osg/Vec4>

#include <string>
#include <vector>
#include <map>

namespace cvr
{

/**
 * @brief Modified version of the OSG StatsHandler that is more tuned to CalVR
 *
 * Several addition stats and stat sub modes have been added.  Also, custom stats can be
 * added by plugins
 */
class CVRKERNEL_EXPORT CVRStatsHandler : public osgGA::GUIEventHandler
{
    public:

        CVRStatsHandler(osgViewer::ViewerBase * viewer);

        enum StatsType
        {
            NO_STATS = 0,
            FRAME_RATE = 1,
            VIEWER_STATS = 2,
            CAMERA_SCENE_STATS = 3,
            VIEWER_SCENE_STATS = 4,
            LAST = 5
        };

        enum StatsSubType
        {
            ALL_SUB_STATS = 0,
            VIEWER_SUB_STATS,
            CAMERA_SUB_STATS,
            PLUGINS_SUB_STATS,
            NO_CUSTOM_ALL_SUB_STATS,
            NO_CUSTOM_VIEWER_SUB_STATS,
            NO_CUSTOM_CAMERA_SUB_STATS,
            NO_CUSTOM_PLUGINS_SUB_STATS,
            LAST_SUB_STATS
        };

        void setKeyEventTogglesOnScreenStats(int key) { _keyEventTogglesOnScreenStats = key; }
        int getKeyEventTogglesOnScreenStats() const { return _keyEventTogglesOnScreenStats; }

        void setKeyEventPrintsOutStats(int key) { _keyEventPrintsOutStats = key; }
        int getKeyEventPrintsOutStats() const { return _keyEventPrintsOutStats; }

        double getBlockMultiplier() const { return _blockMultiplier; }

        void reset();

        osg::Camera* getCamera() { return _camera.get(); }
        const osg::Camera* getCamera() const { return _camera.get(); }

        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

        /** Get the keyboard and mouse usage of this manipulator.*/
        virtual void getUsage(osg::ApplicationUsage& usage) const;

        enum StatAddType
        {
            VIEWER_STAT = 0,
            CAMERA_STAT
        };

        void addStatValue(StatAddType addType, std::string label, std::string statName, osg::Vec3 color, std::string collectName, bool average = false, bool advanced = false);
        void addStatValueWithLine(StatAddType addType, std::string label, std::string statName, osg::Vec3 color, std::string collectName, float lineMax, bool average = false, bool advanced = false);
        void addStatTimeBar(StatAddType addType, std::string label, std::string statDurationName, std::string statStartTimeName, std::string statEndTimeName, osg::Vec3 color, std::string collectName, bool advanced = false);
        void addStatTimeBarWithLine(StatAddType addType, std::string label, std::string statDurationName, std::string statStartTimeName, std::string statEndTimeName, osg::Vec3 color, std::string collectName, float lineMax, bool advanced = false);
        void addStatLine(StatAddType addType, std::string statName, osg::Vec3 color, std::string collectName, float lineMax, bool advanced = false);

        void removeStatValue(std::string statName);
        void removeStatValueWithLine(std::string statName);
        void removeStatTimeBar(std::string statDurationName);
        void removeStatTimeBarWithLine(std::string statDurationName);
        void removeStatLine(std::string statName); 

    protected:

        void setUpHUDCamera(osgViewer::ViewerBase* viewer);

        osg::Geometry* createBackgroundRectangle(const osg::Vec3& pos, const float width, const float height, osg::Vec4& color);

        osg::Geometry* createGeometry(const osg::Vec3& pos, float height, const osg::Vec4& colour, unsigned int numBlocks);

        osg::Geometry* createFrameMarkers(const osg::Vec3& pos, float height, const osg::Vec4& colour, unsigned int numBlocks);

        osg::Geometry* createTick(const osg::Vec3& pos, float height, const osg::Vec4& colour, unsigned int numTicks);

        osg::Node* createCameraTimeStats(std::string& font, osg::Vec3& pos, float startBlocks, bool acquireGPUStats, float characterSize, osg::Stats* viewerStats, osg::Camera* camera);

        void setUpScene(osgViewer::ViewerBase* viewer);

        void updateThreadingModelText();

        struct StatInfo
        {
            osg::Vec4 color;
            osg::Vec4 colorAlpha;
            std::string collectName;
            bool advanced;
        };

        struct StatLineInfo : public StatInfo
        {
            double max;
            std::string name;
        };

        struct StatTimeBarInfo : public StatInfo
        {
            std::string label;
            std::string nameDuration;
            std::string nameTimeStart;
            std::string nameTimeEnd;
        };

        struct StatValueInfo : public StatInfo
        {
            std::string label;
            std::string name;
            bool average;
        };

        void createTimeBar(osg::Stats * viewerStats, osg::Stats * stats, osg::Geode * geode, osg::Vec3 & pos, StatTimeBarInfo * stb, std::string & font, float characterSize, float startBlocks, float leftPos);

        void setCollect(osgViewer::ViewerBase * viewer);
        void calculateStartBlocks(float & startBlocks, float leftPos, float characterSize, std::string & font);
        void refresh();

        std::vector<StatValueInfo*>         _defaultViewerValues;
        std::vector<StatValueInfo*>         _defaultCameraValues;
        std::vector<StatTimeBarInfo*>       _defaultViewerTimeBars;
        std::vector<StatTimeBarInfo*>       _defaultCameraTimeBars;
        std::vector<StatLineInfo*>          _defaultViewerLines;
        std::vector<StatLineInfo*>          _defaultCameraLines;
        std::vector<StatLineInfo*>          _defaultViewerValueLines;
        std::vector<StatLineInfo*>          _defaultCameraValueLines;

        std::vector<StatValueInfo*>         _customViewerValues;
        std::vector<StatValueInfo*>         _customCameraValues;
        std::vector<StatTimeBarInfo*>       _customViewerTimeBars;
        std::vector<StatTimeBarInfo*>       _customCameraTimeBars;
        std::vector<StatLineInfo*>          _customViewerLines;
        std::vector<StatLineInfo*>          _customCameraLines;
        std::vector<StatLineInfo*>          _customViewerValueLines;
        std::vector<StatLineInfo*>          _customCameraValueLines;

        std::map<std::string,bool>          _collectMapViewer;
        std::map<std::string,bool>          _collectMapCameras;


        int                                 _keyEventTogglesOnScreenStats;
        int                                 _keyEventAdvanceSubStats;
        int                                 _keyEventToggleAdvanced;
        int                                 _keyEventPrintsOutStats;

        int                                 _statsType;
        int                                 _statsSubType;

        bool                                _initialized;
        bool                                _advanced;
        bool                                _textCalibrated;
        osg::ref_ptr<osg::Camera>           _camera;

        osg::ref_ptr<osg::Switch>           _switch;

        osgViewer::ViewerBase::ThreadingModel          _threadingModel;
        osg::ref_ptr<osgText::Text>         _threadingModelText;

        unsigned int                        _viewerValuesChildNum;
        unsigned int                        _viewerChildNum;
        unsigned int                        _cameraSceneChildNum;
        unsigned int                        _viewerSceneChildNum;
        unsigned int                        _numBlocks;
        double                              _blockMultiplier;

        float                               _statsWidth;
        float                               _statsHeight;
        float                               _spaceSize;

        osgViewer::ViewerBase *             _viewer;
};

}

#endif
