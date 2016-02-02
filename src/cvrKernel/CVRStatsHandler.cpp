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

#include <cvrKernel/CVRStatsHandler.h>
#include <cvrKernel/CalVR.h>
#include <cvrKernel/PluginManager.h>

#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <iostream>

#include <osgViewer/View>
#include <osgViewer/Viewer>
#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osgViewer/Renderer>
#include <osg/PolygonMode>
#include <osg/Geometry>
#include <osg/BlendFunc>

using namespace cvr;

CVRStatsHandler::CVRStatsHandler(osgViewer::ViewerBase * viewer) :
        _keyEventAdvanceSubStats('A'), _keyEventTogglesOnScreenStats('S'), _keyEventPrintsOutStats(
                'P'), _keyEventToggleAdvanced('t'), _advanced(false), _textCalibrated(
                false), _statsType(NO_STATS), _statsSubType(ALL_SUB_STATS), _initialized(
                false), _threadingModel(osgViewer::ViewerBase::SingleThreaded), _viewerValuesChildNum(
                0), _viewerChildNum(0), _cameraSceneChildNum(0), _viewerSceneChildNum(
                0), _numBlocks(8), _blockMultiplier(10000.0), _statsWidth(
                1280.0f), _statsHeight(1024.0f), _viewer(viewer)
{
    _camera = new osg::Camera;
    _camera->setRenderer(new osgViewer::Renderer(_camera.get()));
    _camera->setProjectionResizePolicy(osg::Camera::FIXED);

    osg::Vec4 colorAdvanced(0.7,0.7,0.7,1.0);
    osg::Vec4 colorAdvancedAlpha = colorAdvanced;
    colorAdvancedAlpha.a() = 0.5;

    StatValueInfo * svi = new StatValueInfo;
    svi->label = "Frame Rate:";
    svi->color = osg::Vec4(1.0,1.0,1.0,1.0);
    svi->colorAlpha = osg::Vec4(1.0,1.0,1.0,0.5);
    svi->name = "Frame rate";
    svi->average = true;
    svi->collectName = "frame_rate";
    svi->advanced = false;
    _defaultViewerValues.push_back(svi);

    StatTimeBarInfo * barInfo = new StatTimeBarInfo;
    barInfo->label = "Event:";
    barInfo->color = osg::Vec4(0.0,1.0,0.5,1.0);
    barInfo->colorAlpha = osg::Vec4(0.0,1.0,0.5,0.5);
    barInfo->nameDuration = "Event traversal time taken";
    barInfo->nameTimeStart = "Event traversal begin time";
    barInfo->nameTimeEnd = "Event traversal end time";
    barInfo->collectName = "event";
    barInfo->advanced = false;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Tracking:";
    barInfo->color = osg::Vec4(0.0,0.5,1.0,1.0);
    barInfo->colorAlpha = osg::Vec4(0.0,0.5,1.0,0.5);
    barInfo->nameDuration = "Tracking time taken";
    barInfo->nameTimeStart = "Tracking begin time";
    barInfo->nameTimeEnd = "Tracking end time";
    barInfo->collectName = "CalVRStats";
    barInfo->advanced = false;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Scene:";
    barInfo->color = colorAdvanced;
    barInfo->colorAlpha = colorAdvancedAlpha;
    barInfo->nameDuration = "Scene time taken";
    barInfo->nameTimeStart = "Scene begin time";
    barInfo->nameTimeEnd = "Scene end time";
    barInfo->collectName = "CalVRStatsAdvanced";
    barInfo->advanced = true;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Menu:";
    barInfo->color = colorAdvanced;
    barInfo->colorAlpha = colorAdvancedAlpha;
    barInfo->nameDuration = "Menu time taken";
    barInfo->nameTimeStart = "Menu begin time";
    barInfo->nameTimeEnd = "Menu end time";
    barInfo->collectName = "CalVRStatsAdvanced";
    barInfo->advanced = true;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Interaction:";
    barInfo->color = osg::Vec4(1.0,0.13,0.67,1.0);
    barInfo->colorAlpha = osg::Vec4(1.0,0.13,0.67,0.5);
    barInfo->nameDuration = "Interaction time taken";
    barInfo->nameTimeStart = "Interaction begin time";
    barInfo->nameTimeEnd = "Interaction end time";
    barInfo->collectName = "CalVRStats";
    barInfo->advanced = false;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Navigation:";
    barInfo->color = colorAdvanced;
    barInfo->colorAlpha = colorAdvancedAlpha;
    barInfo->nameDuration = "Navigation time taken";
    barInfo->nameTimeStart = "Navigation begin time";
    barInfo->nameTimeEnd = "Navigation end time";
    barInfo->collectName = "CalVRStatsAdvanced";
    barInfo->advanced = true;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Collaborative:";
    barInfo->color = colorAdvanced;
    barInfo->colorAlpha = colorAdvancedAlpha;
    barInfo->nameDuration = "Collaborative time taken";
    barInfo->nameTimeStart = "Collaborative begin time";
    barInfo->nameTimeEnd = "Collaborative end time";
    barInfo->collectName = "CalVRStatsAdvanced";
    barInfo->advanced = true;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Threaded Loader:";
    barInfo->color = colorAdvanced;
    barInfo->colorAlpha = colorAdvancedAlpha;
    barInfo->nameDuration = "TLoader time taken";
    barInfo->nameTimeStart = "TLoader begin time";
    barInfo->nameTimeEnd = "TLoader end time";
    barInfo->collectName = "CalVRStatsAdvanced";
    barInfo->advanced = true;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "PreFrame:";
    barInfo->color = osg::Vec4(1.0,0.0,1.0,1.0);
    barInfo->colorAlpha = osg::Vec4(1.0,0.0,1.0,0.5);
    barInfo->nameDuration = "PreFrame time taken";
    barInfo->nameTimeStart = "PreFrame begin time";
    barInfo->nameTimeEnd = "PreFrame end time";
    barInfo->collectName = "CalVRStats";
    barInfo->advanced = false;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Update:";
    barInfo->color = osg::Vec4(0.0,1.0,0.0,1.0);
    barInfo->colorAlpha = osg::Vec4(0.0,1.0,0.0,0.5);
    barInfo->nameDuration = "Update traversal time taken";
    barInfo->nameTimeStart = "Update traversal begin time";
    barInfo->nameTimeEnd = "Update traversal end time";
    barInfo->collectName = "CalVRStats";
    barInfo->advanced = false;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Cluster Sync:";
    barInfo->color = osg::Vec4(1.0,0.0,0.0,1.0);
    barInfo->colorAlpha = osg::Vec4(1.0,0.0,0.0,0.5);
    barInfo->nameDuration = "Cluster Sync time taken";
    barInfo->nameTimeStart = "Cluster Sync begin time";
    barInfo->nameTimeEnd = "Cluster Sync end time";
    barInfo->collectName = "CalVRStats";
    barInfo->advanced = false;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "PostFrame:";
    barInfo->color = colorAdvanced;
    barInfo->colorAlpha = colorAdvancedAlpha;
    barInfo->nameDuration = "PostFrame time taken";
    barInfo->nameTimeStart = "PostFrame begin time";
    barInfo->nameTimeEnd = "PostFrame end time";
    barInfo->collectName = "CalVRStatsAdvanced";
    barInfo->advanced = true;
    _defaultViewerTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Cull:";
    barInfo->color = osg::Vec4(0.0,1.0,1.0,1.0);
    barInfo->colorAlpha = osg::Vec4(0.0,1.0,1.0,0.5);
    barInfo->nameDuration = "Cull traversal time taken";
    barInfo->nameTimeStart = "Cull traversal begin time";
    barInfo->nameTimeEnd = "Cull traversal end time";
    barInfo->collectName = "rendering";
    barInfo->advanced = false;
    _defaultCameraTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Draw:";
    barInfo->color = osg::Vec4(1.0,1.0,0.0,1.0);
    barInfo->colorAlpha = osg::Vec4(1.0,1.0,0.0,0.5);
    barInfo->nameDuration = "Draw traversal time taken";
    barInfo->nameTimeStart = "Draw traversal begin time";
    barInfo->nameTimeEnd = "Draw traversal end time";
    barInfo->collectName = "rendering";
    barInfo->advanced = false;
    _defaultCameraTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Operations:";
    barInfo->color = osg::Vec4(0.7,0.7,0.7,1.0);
    barInfo->colorAlpha = osg::Vec4(0.7,0.7,0.7,0.5);
    barInfo->nameDuration = "Operations time taken";
    barInfo->nameTimeStart = "Operations begin time";
    barInfo->nameTimeEnd = "Operations end time";
    barInfo->collectName = "CalVRRenderingAdvanced";
    barInfo->advanced = true;
    _defaultCameraTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "Finish:";
    barInfo->color = osg::Vec4(0.7,0.7,0.7,1.0);
    barInfo->colorAlpha = osg::Vec4(0.7,0.7,0.7,0.5);
    barInfo->nameDuration = "Finish time taken";
    barInfo->nameTimeStart = "Finish begin time";
    barInfo->nameTimeEnd = "Finish end time";
    barInfo->collectName = "CalVRRenderingAdvanced";
    barInfo->advanced = true;
    _defaultCameraTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "End Barrier:";
    barInfo->color = osg::Vec4(0.7,0.7,0.7,1.0);
    barInfo->colorAlpha = osg::Vec4(0.7,0.7,0.7,0.5);
    barInfo->nameDuration = "End Barrier time taken";
    barInfo->nameTimeStart = "End Barrier begin time";
    barInfo->nameTimeEnd = "End Barrier end time";
    barInfo->collectName = "CalVRRenderingAdvanced";
    barInfo->advanced = true;
    _defaultCameraTimeBars.push_back(barInfo);

    barInfo = new StatTimeBarInfo;
    barInfo->label = "GPU:";
    barInfo->color = osg::Vec4(1.0,0.5,0.0,1.0);
    barInfo->colorAlpha = osg::Vec4(1.0,0.5,0.0,0.5);
    barInfo->nameDuration = "GPU draw time taken";
    barInfo->nameTimeStart = "GPU draw begin time";
    barInfo->nameTimeEnd = "GPU draw end time";
    barInfo->collectName = "gpu";
    barInfo->advanced = false;
    _defaultCameraTimeBars.push_back(barInfo);

    StatLineInfo * sli = new StatLineInfo;
    sli->color = osg::Vec4(1.0,1.0,1.0,1.0);
    sli->colorAlpha = osg::Vec4(1.0,1.0,1.0,0.5);
    sli->max = 100.0;
    sli->name = "Frame rate";
    sli->collectName = "frame_rate";
    sli->advanced = false;
    _defaultViewerValueLines.push_back(sli);

    sli = new StatLineInfo;
    sli->color = osg::Vec4(0.0,1.0,0.5,1.0);
    sli->colorAlpha = osg::Vec4(0.0,1.0,0.5,0.5);
    sli->max = 0.016;
    sli->name = "Event traversal time taken";
    sli->collectName = "event";
    sli->advanced = false;
    _defaultViewerLines.push_back(sli);

    sli = new StatLineInfo;
    sli->color = osg::Vec4(0.0,1.0,0.0,1.0);
    sli->colorAlpha = osg::Vec4(0.0,1.0,0.0,0.5);
    sli->max = 0.016;
    sli->name = "Update traversal time taken";
    sli->collectName = "update";
    sli->advanced = false;
    _defaultViewerLines.push_back(sli);

    sli = new StatLineInfo;
    sli->color = osg::Vec4(0.0,1.0,1.0,1.0);
    sli->colorAlpha = osg::Vec4(0.0,1.0,1.0,0.5);
    sli->max = 0.016;
    sli->name = "Cull traversal time taken";
    sli->collectName = "rendering";
    sli->advanced = false;
    _defaultCameraLines.push_back(sli);

    sli = new StatLineInfo;
    sli->color = osg::Vec4(1.0,1.0,0.0,1.0);
    sli->colorAlpha = osg::Vec4(1.0,1.0,0.0,0.5);
    sli->max = 0.016;
    sli->name = "Draw traversal time taken";
    sli->collectName = "rendering";
    sli->advanced = false;
    _defaultCameraLines.push_back(sli);

    sli = new StatLineInfo;
    sli->color = osg::Vec4(1.0,0.5,0.0,1.0);
    sli->colorAlpha = osg::Vec4(1.0,0.5,0.0,0.5);
    sli->max = 0.016;
    sli->name = "GPU draw time taken";
    sli->collectName = "gpu";
    sli->advanced = false;
    _defaultCameraLines.push_back(sli);
}

bool CVRStatsHandler::handle(const osgGA::GUIEventAdapter& ea,
        osgGA::GUIActionAdapter& aa)
{

    osgViewer::View* myview = dynamic_cast<osgViewer::View*>(&aa);
    if(!myview)
        return false;

    osgViewer::ViewerBase* viewer = myview->getViewerBase();
    if(viewer && _threadingModelText.valid()
            && viewer->getThreadingModel() != _threadingModel)
    {
        _threadingModel = viewer->getThreadingModel();
        updateThreadingModelText();
    }

    if(ea.getHandled())
        return false;

    switch(ea.getEventType())
    {
        case (osgGA::GUIEventAdapter::KEYDOWN):
        {
            if(ea.getKey() == _keyEventTogglesOnScreenStats)
            {
                if(viewer->getViewerStats())
                {
                    if(!_initialized)
                    {
                        setUpHUDCamera(viewer);
                    }

                    if(!_switch)
                    {
                        setUpScene(viewer);
                    }

                    ++_statsType;

                    if(_statsType == LAST)
                        _statsType = NO_STATS;

                    osgViewer::ViewerBase::Cameras cameras;
                    viewer->getCameras(cameras);

                    switch(_statsType)
                    {
                        case (NO_STATS):
                        {
                            for(osgViewer::ViewerBase::Cameras::iterator itr =
                                    cameras.begin(); itr != cameras.end();
                                    ++itr)
                            {
                                osg::Stats* stats = (*itr)->getStats();
                                if(stats)
                                {
                                    stats->collectStats("scene",false);
                                }
                            }

                            viewer->getViewerStats()->collectStats("scene",
                                    false);

                            _camera->setNodeMask(0x0);
                            _switch->setAllChildrenOff();
                            break;
                        }
                        case (FRAME_RATE):
                        {
                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_viewerValuesChildNum,true);
                            break;
                        }
                        case (VIEWER_STATS):
                        {
                            osgViewer::ViewerBase::Scenes scenes;
                            viewer->getScenes(scenes);
                            for(osgViewer::ViewerBase::Scenes::iterator itr =
                                    scenes.begin(); itr != scenes.end(); ++itr)
                            {
                                osgViewer::Scene* scene = *itr;
                                osgDB::DatabasePager* dp =
                                        scene->getDatabasePager();
                                if(dp && dp->isRunning())
                                {
                                    dp->resetStats();
                                }
                            }

                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_viewerChildNum,true);
                            break;
                        }
                        case (CAMERA_SCENE_STATS):
                        {
                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_cameraSceneChildNum,true);

                            for(osgViewer::ViewerBase::Cameras::iterator itr =
                                    cameras.begin(); itr != cameras.end();
                                    ++itr)
                            {
                                osg::Stats* stats = (*itr)->getStats();
                                if(stats)
                                {
                                    stats->collectStats("scene",true);
                                }
                            }

                            break;
                        }
                        case (VIEWER_SCENE_STATS):
                        {
                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_viewerSceneChildNum,true);

                            viewer->getViewerStats()->collectStats("scene",
                                    true);

                            break;
                        }
                        default:
                            break;
                    }

                    setCollect(viewer);

                    aa.requestRedraw();
                }
                return true;
            }
            if(ea.getKey() == _keyEventPrintsOutStats)
            {
                if(viewer->getViewerStats())
                {
                    osg::notify(osg::NOTICE) << std::endl << "Stats report:"
                            << std::endl;
                    typedef std::vector<osg::Stats*> StatsList;
                    StatsList statsList;
                    statsList.push_back(viewer->getViewerStats());

                    osgViewer::ViewerBase::Contexts contexts;
                    viewer->getContexts(contexts);
                    for(osgViewer::ViewerBase::Contexts::iterator gcitr =
                            contexts.begin(); gcitr != contexts.end(); ++gcitr)
                    {
                        osg::GraphicsContext::Cameras& cameras =
                                (*gcitr)->getCameras();
                        for(osg::GraphicsContext::Cameras::iterator itr =
                                cameras.begin(); itr != cameras.end(); ++itr)
                        {
                            if((*itr)->getStats())
                            {
                                statsList.push_back((*itr)->getStats());
                            }
                        }
                    }

                    for(int i =
                            viewer->getViewerStats()->getEarliestFrameNumber();
                            i
                                    <= viewer->getViewerStats()->getLatestFrameNumber()
                                            - 1; ++i)
                    {
                        for(StatsList::iterator itr = statsList.begin();
                                itr != statsList.end(); ++itr)
                        {
                            if(itr == statsList.begin())
                                (*itr)->report(osg::notify(osg::NOTICE),i);
                            else
                                (*itr)->report(osg::notify(osg::NOTICE),i,
                                        "    ");
                        }
                        osg::notify(osg::NOTICE) << std::endl;
                    }

                }
                return true;
            }
            if(ea.getKey() == _keyEventAdvanceSubStats)
            {
                if(_statsType < VIEWER_STATS)
                {
                    return false;
                }

                _statsSubType++;
                if(_statsSubType == LAST_SUB_STATS)
                {
                    _statsSubType = ALL_SUB_STATS;
                }

                if(_switch)
                {
                    _camera->removeChild(_switch);
                    setUpScene(viewer);
                }

                switch(_statsType)
                {
                    case VIEWER_SCENE_STATS:
                        _switch->setValue(_viewerSceneChildNum,true);
                    case CAMERA_SCENE_STATS:
                        _switch->setValue(_cameraSceneChildNum,true);
                    case VIEWER_STATS:
                        _switch->setValue(_viewerChildNum,true);
                    case FRAME_RATE:
                        _switch->setValue(_viewerValuesChildNum,true);
                    case NO_STATS:
                        break;
                    default:
                        break;
                }

                setCollect(viewer);

                aa.requestRedraw();

                return true;
            }

            if(ea.getKey() == _keyEventToggleAdvanced)
            {
                if(_statsType == NO_STATS)
                {
                    return false;
                }

                _advanced = !_advanced;

                if(_switch)
                {
                    _camera->removeChild(_switch);
                    setUpScene(viewer);
                }

                switch(_statsType)
                {
                    case VIEWER_SCENE_STATS:
                        _switch->setValue(_viewerSceneChildNum,true);
                    case CAMERA_SCENE_STATS:
                        _switch->setValue(_cameraSceneChildNum,true);
                    case VIEWER_STATS:
                        _switch->setValue(_viewerChildNum,true);
                    case FRAME_RATE:
                        _switch->setValue(_viewerValuesChildNum,true);
                    case NO_STATS:
                        break;
                    default:
                        break;
                }

                setCollect(viewer);

                aa.requestRedraw();
            }
        }
        default:
            break;
    }

    return false;

}

void CVRStatsHandler::updateThreadingModelText()
{
    switch(_threadingModel)
    {
        case (osgViewer::Viewer::SingleThreaded):
            _threadingModelText->setText("ThreadingModel: SingleThreaded");
            break;
        case (osgViewer::Viewer::CullDrawThreadPerContext):
            _threadingModelText->setText(
                    "ThreadingModel: CullDrawThreadPerContext");
            break;
        case (osgViewer::Viewer::DrawThreadPerContext):
            _threadingModelText->setText(
                    "ThreadingModel: DrawThreadPerContext");
            break;
        case (osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext):
            _threadingModelText->setText(
                    "ThreadingModel: CullThreadPerCameraDrawThreadPerContext");
            break;
        case (osgViewer::Viewer::AutomaticSelection):
            _threadingModelText->setText("ThreadingModel: AutomaticSelection");
            break;
        default:
            _threadingModelText->setText("ThreadingModel: unknown");
            break;
    }
}

void CVRStatsHandler::reset()
{
    _initialized = false;
    _camera->setGraphicsContext(0);
    _camera->removeChildren(0,_camera->getNumChildren());
}

void CVRStatsHandler::setUpHUDCamera(osgViewer::ViewerBase* viewer)
{
    osgViewer::GraphicsWindow* window =
            dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());

    if(!window)
    {
        osgViewer::Viewer::Windows windows;
        viewer->getWindows(windows);

        if(windows.empty())
            return;

        window = windows.front();
    }

    _camera->setGraphicsContext(window);

    _camera->setViewport(0,0,window->getTraits()->width,
            window->getTraits()->height);

    _camera->setRenderOrder(osg::Camera::POST_RENDER,10);

    _camera->setProjectionMatrix(
            osg::Matrix::ortho2D(0.0,_statsWidth,0.0,_statsHeight));
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    _camera->setClearMask(0);

    _camera->setRenderer(new osgViewer::Renderer(_camera.get()));

    osg::StateSet * stateset = _camera->getOrCreateStateSet();
    osg::BlendFunc * bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    stateset->setAttributeAndModes(bf,osg::StateAttribute::ON);

    _initialized = true;
}

// Drawcallback to draw averaged attribute
struct AveragedValueTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
        AveragedValueTextDrawCallback(osg::Stats* stats,
                const std::string& name, int frameDelta,
                bool averageInInverseSpace, double multiplier) :
                _stats(stats), _attributeName(name), _frameDelta(frameDelta), _averageInInverseSpace(
                        averageInInverseSpace), _multiplier(multiplier), _tickLastUpdated(
                        0)
        {
        }

        /** do customized draw code.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo,
                const osg::Drawable* drawable) const
        {
            osgText::Text* text = (osgText::Text*)drawable;

            osg::Timer_t tick = osg::Timer::instance()->tick();
            double delta = osg::Timer::instance()->delta_m(_tickLastUpdated,
                    tick);

            if(delta > 50) // update every 50ms
            {
                _tickLastUpdated = tick;
                double value;
                if(_stats->getAveragedAttribute(_attributeName,value,
                        _averageInInverseSpace))
                {
                    sprintf(_tmpText,"%4.3f",value * _multiplier);
                    text->setText(_tmpText);
                }
                else
                {
                    text->setText("");
                }
            }
            text->drawImplementation(renderInfo);
        }

        osg::ref_ptr<osg::Stats> _stats;
        std::string _attributeName;
        int _frameDelta;
        bool _averageInInverseSpace;
        double _multiplier;
        mutable char _tmpText[128];
        mutable osg::Timer_t _tickLastUpdated;
};

struct CameraSceneStatsTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
        CameraSceneStatsTextDrawCallback(osg::Camera* camera, int cameraNumber) :
                _camera(camera), _tickLastUpdated(0), _cameraNumber(
                        cameraNumber)
        {
        }

        /** do customized draw code.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo,
                const osg::Drawable* drawable) const
        {
            if(!_camera)
                return;

            osgText::Text* text = (osgText::Text*)drawable;

            osg::Timer_t tick = osg::Timer::instance()->tick();
            double delta = osg::Timer::instance()->delta_m(_tickLastUpdated,
                    tick);

            if(delta > 100) // update every 100ms
            {
                _tickLastUpdated = tick;
                std::ostringstream viewStr;
                viewStr.clear();

                osg::Stats* stats = _camera->getStats();
                osgViewer::Renderer* renderer =
                        dynamic_cast<osgViewer::Renderer*>(_camera->getRenderer());

                if(stats && renderer)
                {
                    viewStr.setf(std::ios::left,std::ios::adjustfield);
                    viewStr.width(14);
                    // Used fixed formatting, as scientific will switch to "...e+.." notation for
                    // large numbers of vertices/drawables/etc.
                    viewStr.setf(std::ios::fixed);
                    viewStr.precision(0);

                    viewStr << std::setw(1) << "#" << _cameraNumber
                            << std::endl;

                    // Camera name
                    if(!_camera->getName().empty())
                        viewStr << _camera->getName();
                    viewStr << std::endl;

                    int frameNumber =
                            renderInfo.getState()->getFrameStamp()->getFrameNumber();
                    if(!(renderer->getGraphicsThreadDoesCull()))
                    {
                        --frameNumber;
                    }

#define STATS_ATTRIBUTE(str) \
                    if (stats->getAttribute(frameNumber, str, value)) \
                        viewStr << std::setw(8) << value << std::endl; \
                    else \
                        viewStr << std::setw(8) << "." << std::endl; \

                    double value = 0.0;

                    STATS_ATTRIBUTE("Visible number of lights")
                    STATS_ATTRIBUTE("Visible number of render bins")
                    STATS_ATTRIBUTE("Visible depth")
                    STATS_ATTRIBUTE("Visible number of materials")
                    STATS_ATTRIBUTE("Visible number of impostors")
                    STATS_ATTRIBUTE("Visible number of drawables")
                    STATS_ATTRIBUTE("Visible vertex count")

                    STATS_ATTRIBUTE("Visible number of GL_POINTS")
                    STATS_ATTRIBUTE("Visible number of GL_LINES")
                    STATS_ATTRIBUTE("Visible number of GL_LINE_STRIP")
                    STATS_ATTRIBUTE("Visible number of GL_LINE_LOOP")
                    STATS_ATTRIBUTE("Visible number of GL_TRIANGLES")
                    STATS_ATTRIBUTE("Visible number of GL_TRIANGLE_STRIP")
                    STATS_ATTRIBUTE("Visible number of GL_TRIANGLE_FAN")
                    STATS_ATTRIBUTE("Visible number of GL_QUADS")
                    STATS_ATTRIBUTE("Visible number of GL_QUAD_STRIP")
                    STATS_ATTRIBUTE("Visible number of GL_POLYGON")

                    text->setText(viewStr.str());
                }
            }
            text->drawImplementation(renderInfo);
        }

        osg::observer_ptr<osg::Camera> _camera;
        mutable osg::Timer_t _tickLastUpdated;
        int _cameraNumber;
};

struct ViewSceneStatsTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
        ViewSceneStatsTextDrawCallback(osgViewer::View* view, int viewNumber) :
                _view(view), _tickLastUpdated(0), _viewNumber(viewNumber)
        {
        }

        /** do customized draw code.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo,
                const osg::Drawable* drawable) const
        {
            if(!_view)
                return;

            osgText::Text* text = (osgText::Text*)drawable;

            osg::Timer_t tick = osg::Timer::instance()->tick();
            double delta = osg::Timer::instance()->delta_m(_tickLastUpdated,
                    tick);

            if(delta > 200) // update every 100ms
            {
                _tickLastUpdated = tick;
                osg::Stats* stats = _view->getStats();
                if(stats)
                {
                    std::ostringstream viewStr;
                    viewStr.clear();
                    viewStr.setf(std::ios::left,std::ios::adjustfield);
                    viewStr.width(20);
                    viewStr.setf(std::ios::fixed);
                    viewStr.precision(0);

                    viewStr << std::setw(1) << "#" << _viewNumber;

                    // View name
                    if(!_view->getName().empty())
                        viewStr << ": " << _view->getName();
                    viewStr << std::endl;

                    int frameNumber =
                            renderInfo.getState()->getFrameStamp()->getFrameNumber();
                    // if (!(renderer->getGraphicsThreadDoesCull()))
                    {
                        --frameNumber;
                    }

#define STATS_ATTRIBUTE_PAIR(str1, str2) \
                    if (stats->getAttribute(frameNumber, str1, value)) \
                        viewStr << std::setw(9) << value; \
                    else \
                        viewStr << std::setw(9) << "."; \
                    if (stats->getAttribute(frameNumber, str2, value)) \
                        viewStr << std::setw(9) << value << std::endl; \
                    else \
                        viewStr << std::setw(9) << "." << std::endl; \

                    double value = 0.0;

                    // header
                    viewStr << std::setw(9) << "Unique" << std::setw(9)
                            << "Instance" << std::endl;

                    STATS_ATTRIBUTE_PAIR("Number of unique StateSet",
                            "Number of instanced Stateset")
                    STATS_ATTRIBUTE_PAIR("Number of unique Group",
                            "Number of instanced Group")
                    STATS_ATTRIBUTE_PAIR("Number of unique Transform",
                            "Number of instanced Transform")
                    STATS_ATTRIBUTE_PAIR("Number of unique LOD",
                            "Number of instanced LOD")
                    STATS_ATTRIBUTE_PAIR("Number of unique Switch",
                            "Number of instanced Switch")
                    STATS_ATTRIBUTE_PAIR("Number of unique Geode",
                            "Number of instanced Geode")
                    STATS_ATTRIBUTE_PAIR("Number of unique Drawable",
                            "Number of instanced Drawable")
                    STATS_ATTRIBUTE_PAIR("Number of unique Geometry",
                            "Number of instanced Geometry")
                    STATS_ATTRIBUTE_PAIR("Number of unique Vertices",
                            "Number of instanced Vertices")
                    STATS_ATTRIBUTE_PAIR("Number of unique Primitives",
                            "Number of instanced Primitives")

                    text->setText(viewStr.str());
                }
                else
                {
                    OSG_NOTIFY(osg::WARN)<<std::endl<<"No valid view to collect scene stats from"<<std::endl;

                    text->setText("");
                }
            }
            text->drawImplementation(renderInfo);
        }

        osg::observer_ptr<osgViewer::View> _view;
        mutable osg::Timer_t _tickLastUpdated;
        int _viewNumber;
};

struct BlockDrawCallback : public virtual osg::Drawable::DrawCallback
{
        BlockDrawCallback(CVRStatsHandler* statsHandler, float xPos,
                osg::Stats* viewerStats, osg::Stats* stats,
                const std::string& beginName, const std::string& endName,
                int frameDelta, int numFrames) :
                _statsHandler(statsHandler), _xPos(xPos), _viewerStats(
                        viewerStats), _stats(stats), _beginName(beginName), _endName(
                        endName), _frameDelta(frameDelta), _numFrames(numFrames)
        {
        }

        /** do customized draw code.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo,
                const osg::Drawable* drawable) const
        {
            osg::Geometry* geom = (osg::Geometry*)drawable;
            osg::Vec3Array* vertices = (osg::Vec3Array*)geom->getVertexArray();

            int frameNumber =
                    renderInfo.getState()->getFrameStamp()->getFrameNumber();

            int startFrame = frameNumber + _frameDelta - _numFrames + 1;
            int endFrame = frameNumber + _frameDelta;
            double referenceTime;
            if(!_viewerStats->getAttribute(startFrame,"Reference time",
                    referenceTime))
            {
                return;
            }

            unsigned int vi = 0;
            double beginValue, endValue;
            for(int i = startFrame; i <= endFrame; ++i)
            {
                if(_stats->getAttribute(i,_beginName,beginValue)
                        && _stats->getAttribute(i,_endName,endValue))
                {
                    (*vertices)[vi++].x() = _xPos
                            + (beginValue - referenceTime)
                                    * _statsHandler->getBlockMultiplier();
                    (*vertices)[vi++].x() = _xPos
                            + (beginValue - referenceTime)
                                    * _statsHandler->getBlockMultiplier();
                    (*vertices)[vi++].x() = _xPos
                            + (endValue - referenceTime)
                                    * _statsHandler->getBlockMultiplier();
                    (*vertices)[vi++].x() = _xPos
                            + (endValue - referenceTime)
                                    * _statsHandler->getBlockMultiplier();
                }
            }

            osg::DrawArrays* drawArrays =
                    static_cast<osg::DrawArrays*>(geom->getPrimitiveSet(0));
            drawArrays->setCount(vi);

            drawable->drawImplementation(renderInfo);
        }

        CVRStatsHandler* _statsHandler;
        float _xPos;
        osg::ref_ptr<osg::Stats> _viewerStats;
        osg::ref_ptr<osg::Stats> _stats;
        std::string _beginName;
        std::string _endName;
        int _frameDelta;
        int _numFrames;
};

osg::Geometry* CVRStatsHandler::createBackgroundRectangle(const osg::Vec3& pos,
        const float width, const float height, osg::Vec4& color)
{
    osg::StateSet *ss = new osg::StateSet;

    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);
    geometry->setStateSet(ss);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);

    vertices->push_back(osg::Vec3(pos.x(),pos.y(),0));
    vertices->push_back(osg::Vec3(pos.x(),pos.y() - height,0));
    vertices->push_back(osg::Vec3(pos.x() + width,pos.y() - height,0));
    vertices->push_back(osg::Vec3(pos.x() + width,pos.y(),0));

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::DrawElementsUInt *base = new osg::DrawElementsUInt(
            osg::PrimitiveSet::QUADS,0);
    base->push_back(0);
    base->push_back(1);
    base->push_back(2);
    base->push_back(3);

    geometry->addPrimitiveSet(base);

    return geometry;
}

struct StatsGraph : public osg::MatrixTransform
{
        StatsGraph(osg::Vec3 pos, float width, float height) :
                _pos(pos), _width(width), _height(height), _statsGraphGeode(
                        new osg::Geode)
        {
            _pos -= osg::Vec3(0,height,0.1);
            setMatrix(osg::Matrix::translate(_pos));
            addChild(_statsGraphGeode.get());
        }

        void addStatGraph(osg::Stats* viewerStats, osg::Stats* stats,
                const osg::Vec4& color, float max, const std::string& nameBegin,
                const std::string& nameEnd = "")
        {
            _statsGraphGeode->addDrawable(
                    new Graph(_width,_height,viewerStats,stats,color,max,
                            nameBegin,nameEnd));
        }

        osg::Vec3 _pos;
        float _width;
        float _height;

        osg::ref_ptr<osg::Geode> _statsGraphGeode;

    protected:
        struct Graph : public osg::Geometry
        {
                Graph(float width, float height, osg::Stats* viewerStats,
                        osg::Stats* stats, const osg::Vec4& color, float max,
                        const std::string& nameBegin,
                        const std::string& nameEnd = "")
                {
                    setUseDisplayList(false);

                    setVertexArray(new osg::Vec3Array);

                    osg::Vec4Array* colors = new osg::Vec4Array;
                    colors->push_back(color);
                    setColorArray(colors);
                    setColorBinding(osg::Geometry::BIND_OVERALL);

                    setDrawCallback(
                            new GraphUpdateCallback(width,height,viewerStats,
                                    stats,max,nameBegin,nameEnd));
                }
        };

        struct GraphUpdateCallback : public osg::Drawable::DrawCallback
        {
                GraphUpdateCallback(float width, float height,
                        osg::Stats* viewerStats, osg::Stats* stats, float max,
                        const std::string& nameBegin,
                        const std::string& nameEnd = "") :
                        _width((unsigned int)width), _height(
                                (unsigned int)height), _curX(0), _viewerStats(
                                viewerStats), _stats(stats), _max(max), _nameBegin(
                                nameBegin), _nameEnd(nameEnd)
                {
                }

                virtual void drawImplementation(osg::RenderInfo& renderInfo,
                        const osg::Drawable* drawable) const
                {
                    osg::Geometry* geometry =
                            const_cast<osg::Geometry*>(drawable->asGeometry());
                    if(!geometry)
                        return;
                    osg::Vec3Array* vertices =
                            dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                    if(!vertices)
                        return;

                    int frameNumber =
                            renderInfo.getState()->getFrameStamp()->getFrameNumber();

                    // Get stats
                    double value;
                    if(_nameEnd.empty())
                    {
                        if(!_stats->getAveragedAttribute(_nameBegin,value,true))
                        {
                            value = 0.0;
                        }
                    }
                    else
                    {
                        double beginValue, endValue;
                        if(_stats->getAttribute(frameNumber,_nameBegin,
                                beginValue)
                                && _stats->getAttribute(frameNumber,_nameEnd,
                                        endValue))
                        {
                            value = endValue - beginValue;
                        }
                        else
                        {
                            value = 0.0;
                        }
                    }

                    // Add new vertex for this frame.
                    value = osg::clampTo(value,0.0,double(_max));
                    vertices->push_back(
                            osg::Vec3(float(_curX),
                                    float(_height) / _max * value,0));

                    // One vertex per pixel in X.
                    if(vertices->size() > _width)
                    {
                        unsigned int excedent = vertices->size() - _width;
                        vertices->erase(vertices->begin(),
                                vertices->begin() + excedent);

                        // Make the graph scroll when there is enough data.
                        // Note: We check the frame number so that even if we have
                        // many graphs, the transform is translated only once per
                        // frame.
                        static const float increment = -1.0;
                        if(GraphUpdateCallback::_frameNumber != frameNumber)
                        {
                            // We know the exact layout of this part of the scene
                            // graph, so this is OK...
                            osg::MatrixTransform* transform =
                                    geometry->getParent(0)->getParent(0)->asTransform()->asMatrixTransform();
                            if(transform)
                            {
                                transform->setMatrix(
                                        transform->getMatrix()
                                                * osg::Matrix::translate(
                                                        osg::Vec3(increment,0,
                                                                0)));
                            }
                        }
                    }
                    else
                    {
                        // Create primitive set if none exists.
                        if(geometry->getNumPrimitiveSets() == 0)
                            geometry->addPrimitiveSet(
                                    new osg::DrawArrays(GL_LINE_STRIP,0,0));

                        // Update primitive set.
                        osg::DrawArrays* drawArrays =
                                dynamic_cast<osg::DrawArrays*>(geometry->getPrimitiveSet(
                                        0));
                        if(!drawArrays)
                            return;
                        drawArrays->setFirst(0);
                        drawArrays->setCount(vertices->size());
                    }

                    _curX++;
                    GraphUpdateCallback::_frameNumber = frameNumber;

                    geometry->dirtyBound();

                    drawable->drawImplementation(renderInfo);
                }

                const unsigned int _width;
                const unsigned int _height;
                mutable unsigned int _curX;
                osg::Stats* _viewerStats;
                osg::Stats* _stats;
                const float _max;
                const std::string _nameBegin;
                const std::string _nameEnd;
                static int _frameNumber;
        };
};

int StatsGraph::GraphUpdateCallback::_frameNumber = 0;

osg::Geometry* CVRStatsHandler::createGeometry(const osg::Vec3& pos,
        float height, const osg::Vec4& colour, unsigned int numBlocks)
{
    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    vertices->reserve(numBlocks * 4);

    for(unsigned int i = 0; i < numBlocks; ++i)
    {
        vertices->push_back(pos + osg::Vec3(i * 20,height,0.0));
        vertices->push_back(pos + osg::Vec3(i * 20,0.0,0.0));
        vertices->push_back(pos + osg::Vec3(i * 20 + 10.0,0.0,0.0));
        vertices->push_back(pos + osg::Vec3(i * 20 + 10.0,height,0.0));
    }

    osg::Vec4Array* colours = new osg::Vec4Array;
    colours->push_back(colour);
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,numBlocks * 4));

    return geometry;
}

struct FrameMarkerDrawCallback : public virtual osg::Drawable::DrawCallback
{
        FrameMarkerDrawCallback(CVRStatsHandler* statsHandler, float xPos,
                osg::Stats* viewerStats, int frameDelta, int numFrames) :
                _statsHandler(statsHandler), _xPos(xPos), _viewerStats(
                        viewerStats), _frameDelta(frameDelta), _numFrames(
                        numFrames)
        {
        }

        /** do customized draw code.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo,
                const osg::Drawable* drawable) const
        {
            osg::Geometry* geom = (osg::Geometry*)drawable;
            osg::Vec3Array* vertices = (osg::Vec3Array*)geom->getVertexArray();

            int frameNumber =
                    renderInfo.getState()->getFrameStamp()->getFrameNumber();

            int startFrame = frameNumber + _frameDelta - _numFrames + 1;
            int endFrame = frameNumber + _frameDelta;
            double referenceTime;
            if(!_viewerStats->getAttribute(startFrame,"Reference time",
                    referenceTime))
            {
                return;
            }

            unsigned int vi = 0;
            double currentReferenceTime;
            for(int i = startFrame; i <= endFrame; ++i)
            {
                if(_viewerStats->getAttribute(i,"Reference time",
                        currentReferenceTime))
                {
                    (*vertices)[vi++].x() = _xPos
                            + (currentReferenceTime - referenceTime)
                                    * _statsHandler->getBlockMultiplier();
                    (*vertices)[vi++].x() = _xPos
                            + (currentReferenceTime - referenceTime)
                                    * _statsHandler->getBlockMultiplier();
                }
            }

            drawable->drawImplementation(renderInfo);
        }

        CVRStatsHandler* _statsHandler;
        float _xPos;
        osg::ref_ptr<osg::Stats> _viewerStats;
        std::string _endName;
        int _frameDelta;
        int _numFrames;
};

struct PagerCallback : public virtual osg::NodeCallback
{

        PagerCallback(osgDB::DatabasePager* dp, osgText::Text* minValue,
                osgText::Text* maxValue, osgText::Text* averageValue,
                osgText::Text* filerequestlist, osgText::Text* compilelist,
                double multiplier) :
                _dp(dp), _minValue(minValue), _maxValue(maxValue), _averageValue(
                        averageValue), _filerequestlist(filerequestlist), _compilelist(
                        compilelist), _multiplier(multiplier)
        {
        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            if(_dp.valid())
            {
                double value = _dp->getAverageTimeToMergeTiles();
                if(value >= 0.0 && value <= 1000)
                {
                    sprintf(_tmpText,"%4.0f",value * _multiplier);
                    _averageValue->setText(_tmpText);
                }
                else
                {
                    _averageValue->setText("");
                }

                value = _dp->getMinimumTimeToMergeTile();
                if(value >= 0.0 && value <= 1000)
                {
                    sprintf(_tmpText,"%4.0f",value * _multiplier);
                    _minValue->setText(_tmpText);
                }
                else
                {
                    _minValue->setText("");
                }

                value = _dp->getMaximumTimeToMergeTile();
                if(value >= 0.0 && value <= 1000)
                {
                    sprintf(_tmpText,"%4.0f",value * _multiplier);
                    _maxValue->setText(_tmpText);
                }
                else
                {
                    _maxValue->setText("");
                }

                sprintf(_tmpText,"%4d",_dp->getFileRequestListSize());
                _filerequestlist->setText(_tmpText);

                sprintf(_tmpText,"%4d",_dp->getDataToCompileListSize());
                _compilelist->setText(_tmpText);
            }

            traverse(node,nv);
        }

        osg::observer_ptr<osgDB::DatabasePager> _dp;

        osg::ref_ptr<osgText::Text> _minValue;
        osg::ref_ptr<osgText::Text> _maxValue;
        osg::ref_ptr<osgText::Text> _averageValue;
        osg::ref_ptr<osgText::Text> _filerequestlist;
        osg::ref_ptr<osgText::Text> _compilelist;
        double _multiplier;
        char _tmpText[128];
        osg::Timer_t _tickLastUpdated;
};

osg::Geometry* CVRStatsHandler::createFrameMarkers(const osg::Vec3& pos,
        float height, const osg::Vec4& colour, unsigned int numBlocks)
{
    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    vertices->reserve(numBlocks * 2);

    for(unsigned int i = 0; i < numBlocks; ++i)
    {
        vertices->push_back(
                pos
                        + osg::Vec3(double(i) * _blockMultiplier * 0.01,height,
                                0.0));
        vertices->push_back(
                pos + osg::Vec3(double(i) * _blockMultiplier * 0.01,0.0,0.0));
    }

    osg::Vec4Array* colours = new osg::Vec4Array;
    colours->push_back(colour);
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,numBlocks * 2));

    return geometry;
}

osg::Geometry* CVRStatsHandler::createTick(const osg::Vec3& pos, float height,
        const osg::Vec4& colour, unsigned int numTicks)
{
    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    vertices->reserve(numTicks * 2);

    for(unsigned int i = 0; i < numTicks; ++i)
    {
        float tickHeight = (i % 10) ? height : height * 2.0;
        vertices->push_back(
                pos
                        + osg::Vec3(double(i) * _blockMultiplier * 0.001,
                                tickHeight,0.0));
        vertices->push_back(
                pos + osg::Vec3(double(i) * _blockMultiplier * 0.001,0.0,0.0));
    }

    osg::Vec4Array* colours = new osg::Vec4Array;
    colours->push_back(colour);
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,numTicks * 2));

    return geometry;
}

void CVRStatsHandler::setUpScene(osgViewer::ViewerBase* viewer)
{
    _switch = new osg::Switch;

    _camera->addChild(_switch.get());

    osg::StateSet* stateset = _switch->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setAttribute(new osg::PolygonMode(),
            osg::StateAttribute::PROTECTED);

    std::string font(
            CalVR::instance()->getResourceDir() + "/resources/arial.ttf");

    // collect all the relevant cameras
    osgViewer::ViewerBase::Cameras validCameras;
    viewer->getCameras(validCameras);

    osgViewer::ViewerBase::Cameras cameras;
    for(osgViewer::ViewerBase::Cameras::iterator itr = validCameras.begin();
            itr != validCameras.end(); ++itr)
    {
        if((*itr)->getStats())
        {
            cameras.push_back(*itr);
        }
    }

    // check for query time support
    unsigned int numCamrasWithTimerQuerySupport = 0;
    for(osgViewer::ViewerBase::Cameras::iterator citr = cameras.begin();
            citr != cameras.end(); ++citr)
    {
        if((*citr)->getGraphicsContext())
        {
            unsigned int contextID =
                    (*citr)->getGraphicsContext()->getState()->getContextID();
            const osg::Drawable::Extensions* extensions =
                    osg::Drawable::getExtensions(contextID,false);
            if(extensions && extensions->isTimerQuerySupported())
            {
                ++numCamrasWithTimerQuerySupport;
            }
        }
    }

    bool acquireGPUStats = numCamrasWithTimerQuerySupport == cameras.size();

    float leftPos = 10.0f;
    float startBlocks = 150.0f;
    float characterSize = 20.0f;

    if(!_textCalibrated)
    {
        osg::ref_ptr<osgText::Text> ctext = new osgText::Text();
        ctext->setFont(font);
        ctext->setCharacterSize(characterSize);
        ctext->setText("::");

        float ctwidth = ctext->getBound().xMax() - ctext->getBound().xMin();

        ctext->setText(": :");
        _spaceSize = (ctext->getBound().xMax() - ctext->getBound().xMin())
                - ctwidth;

        _textCalibrated = true;
    }

    calculateStartBlocks(startBlocks,leftPos,characterSize,font);

    osg::Vec3 pos(leftPos,_statsHeight - 24.0f,0.0f);

    osg::Vec4 colorDP(1.0f,1.0f,0.5f,1.0f);

    osg::Group * viewerValuesGroup = new osg::Group();
    _viewerValuesChildNum = _switch->getNumChildren();
    _switch->addChild(viewerValuesGroup,false);

    for(int i = 0; i < _defaultViewerValues.size(); i++)
    {
	pos.x() = leftPos;

        osg::Geode* geode = new osg::Geode();
        viewerValuesGroup->addChild(geode);

        osg::ref_ptr<osgText::Text> label = new osgText::Text;
        geode->addDrawable(label.get());

        label->setColor(_defaultViewerValues[i]->color);
        label->setFont(font);
        label->setCharacterSize(characterSize);
        label->setPosition(pos);
        label->setText(_defaultViewerValues[i]->label);

        pos.x() = label->getBound().xMax() + _spaceSize;

        osg::ref_ptr<osgText::Text> value = new osgText::Text;
        geode->addDrawable(value.get());

        value->setColor(_defaultViewerValues[i]->color);
        value->setFont(font);
        value->setCharacterSize(characterSize);
        value->setPosition(pos);
        value->setText("0.0");

        value->setDrawCallback(
                new AveragedValueTextDrawCallback(viewer->getViewerStats(),
                        _defaultViewerValues[i]->name,-1,true,1.0));

        pos.y() -= characterSize * 1.5f;
    }

    for(int i = 0; i < _customViewerValues.size(); i++)
    {
	pos.x() = leftPos;

        osg::Geode* geode = new osg::Geode();
        viewerValuesGroup->addChild(geode);

        osg::ref_ptr<osgText::Text> label = new osgText::Text;
        geode->addDrawable(label.get());

        label->setColor(_customViewerValues[i]->color);
        label->setFont(font);
        label->setCharacterSize(characterSize);
        label->setPosition(pos);
        label->setText(_customViewerValues[i]->label);

        pos.x() = label->getBound().xMax() + _spaceSize;

        osg::ref_ptr<osgText::Text> value = new osgText::Text;
        geode->addDrawable(value.get());

        value->setColor(_customViewerValues[i]->color);
        value->setFont(font);
        value->setCharacterSize(characterSize);
        value->setPosition(pos);
        value->setText("0.0");

        value->setDrawCallback(
                new AveragedValueTextDrawCallback(viewer->getViewerStats(),
                        _customViewerValues[i]->name,-1,true,1.0));

        pos.y() -= characterSize * 1.5f;
    }

    for(osgViewer::ViewerBase::Cameras::iterator citr = cameras.begin();
            citr != cameras.end(); ++citr)
    {
        for(int i = 0; i < _defaultCameraValues.size(); i++)
        {
	    pos.x() = leftPos;

            osg::Geode* geode = new osg::Geode();
            viewerValuesGroup->addChild(geode);

            osg::ref_ptr<osgText::Text> label = new osgText::Text;
            geode->addDrawable(label.get());

            label->setColor(_defaultCameraValues[i]->color);
            label->setFont(font);
            label->setCharacterSize(characterSize);
            label->setPosition(pos);
            label->setText(_defaultCameraValues[i]->label);

            pos.x() = label->getBound().xMax() + _spaceSize;

            osg::ref_ptr<osgText::Text> value = new osgText::Text;
            geode->addDrawable(value.get());

            value->setColor(_defaultCameraValues[i]->color);
            value->setFont(font);
            value->setCharacterSize(characterSize);
            value->setPosition(pos);
            value->setText("0.0");

            value->setDrawCallback(
                    new AveragedValueTextDrawCallback((*citr)->getStats(),
                            _defaultCameraValues[i]->name,-1,true,1.0));

            pos.y() -= characterSize * 1.5f;
        }

        for(int i = 0; i < _customCameraValues.size(); i++)
        {
	    pos.x() = leftPos;

            osg::Geode* geode = new osg::Geode();
            viewerValuesGroup->addChild(geode);

            osg::ref_ptr<osgText::Text> label = new osgText::Text;
            geode->addDrawable(label.get());

            label->setColor(_customCameraValues[i]->color);
            label->setFont(font);
            label->setCharacterSize(characterSize);
            label->setPosition(pos);
            label->setText(_customCameraValues[i]->label);

            pos.x() = label->getBound().xMax() + _spaceSize;

            osg::ref_ptr<osgText::Text> value = new osgText::Text;
            geode->addDrawable(value.get());

            value->setColor(_customCameraValues[i]->color);
            value->setFont(font);
            value->setCharacterSize(characterSize);
            value->setPosition(pos);
            value->setText("0.0");

            value->setDrawCallback(
                    new AveragedValueTextDrawCallback((*citr)->getStats(),
                            _customCameraValues[i]->name,-1,true,1.0));

            pos.y() -= characterSize * 1.5f;
        }
    }

    osg::Vec4 backgroundColor(0.0,0.0,0.0f,0.3);
    osg::Vec4 staticTextColor(1.0,1.0,0.0f,1.0);
    osg::Vec4 dynamicTextColor(1.0,1.0,1.0f,1.0);
    float backgroundMargin = 5;
    float backgroundSpacing = 3;

    // viewer stats
    {
        osg::Group* group = new osg::Group;
        _viewerChildNum = _switch->getNumChildren();
        _switch->addChild(group,false);

        osg::Geode* geode = new osg::Geode();
        group->addChild(geode);

        {
            pos.x() = leftPos;

            _threadingModelText = new osgText::Text;
            geode->addDrawable(_threadingModelText.get());

            _threadingModelText->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
            _threadingModelText->setFont(font);
            _threadingModelText->setCharacterSize(characterSize);
            _threadingModelText->setPosition(pos);

            updateThreadingModelText();

            pos.y() -= characterSize * 1.5f;
        }

        float topOfViewerStats = pos.y() + characterSize;

        float topY = pos.y() + characterSize + backgroundMargin;
        osg::Vec3 posOld = pos;

        if(_statsSubType == ALL_SUB_STATS || _statsSubType == VIEWER_SUB_STATS
                || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
                || _statsSubType == NO_CUSTOM_VIEWER_SUB_STATS)
        {
            for(int i = 0; i < _defaultViewerTimeBars.size(); i++)
            {
                if(_defaultViewerTimeBars[i]->advanced && !_advanced)
                {
                    continue;
                }
                createTimeBar(viewer->getViewerStats(),NULL,geode,pos,
                        _defaultViewerTimeBars[i],font,characterSize,
                        startBlocks,leftPos);
            }
        }

        if(_statsSubType == ALL_SUB_STATS || _statsSubType == VIEWER_SUB_STATS)
        {
            for(int i = 0; i < _customViewerTimeBars.size(); i++)
            {
                if(_customViewerTimeBars[i]->advanced && !_advanced)
                {
                    continue;
                }
                createTimeBar(viewer->getViewerStats(),NULL,geode,pos,
                        _customViewerTimeBars[i],font,characterSize,startBlocks,
                        leftPos);
            }
        }

        if(_statsSubType == PLUGINS_SUB_STATS
                || _statsSubType == NO_CUSTOM_PLUGINS_SUB_STATS)
        {
            StatTimeBarInfo sbi;
            sbi.collectName = "CalVRStatsPlugins";
            sbi.advanced = false;

            std::vector<std::string> pluginList =
                    PluginManager::instance()->getLoadedPluginList();
            for(int i = 0; i < pluginList.size(); i++)
            {
                sbi.label = pluginList[i] + ": ";
                sbi.color = osg::Vec4(0.0,1.0,0.5,1.0);
                sbi.colorAlpha = sbi.color;
                sbi.colorAlpha.a() = 0.5;
                sbi.nameDuration = pluginList[i] + " preFrame time taken";
                sbi.nameTimeStart = pluginList[i] + " preFrame begin time";
                sbi.nameTimeEnd = pluginList[i] + " preFrame end time";

                createTimeBar(viewer->getViewerStats(),NULL,geode,pos,&sbi,font,
                        characterSize,startBlocks,leftPos);
            }
        }

        pos.x() = leftPos;

        // add camera stats
        for(osgViewer::ViewerBase::Cameras::iterator citr = cameras.begin();
                citr != cameras.end(); ++citr)
        {
            group->addChild(
                    createCameraTimeStats(font,pos,startBlocks,acquireGPUStats,
                            characterSize,viewer->getViewerStats(),*citr));
        }

        geode->addDrawable(
                createBackgroundRectangle(
                        posOld
                                + osg::Vec3(-backgroundMargin,
                                        characterSize + backgroundMargin,0),
                        _statsWidth - 2 * backgroundMargin,
                        topY - (pos.y() + characterSize - backgroundMargin),
                        backgroundColor));

        // add frame ticks
        {
            osg::Geode* geode = new osg::Geode;
            group->addChild(geode);

            osg::Vec4 colourTicks(1.0f,1.0f,1.0f,0.5f);

            pos.x() = startBlocks;
            pos.y() += characterSize;
            float height = topOfViewerStats - pos.y();

            osg::Geometry* ticks = createTick(pos,5.0f,colourTicks,100);
            geode->addDrawable(ticks);

            osg::Geometry* frameMarkers = createFrameMarkers(pos,height,
                    colourTicks,_numBlocks + 1);
            frameMarkers->setDrawCallback(
                    new FrameMarkerDrawCallback(this,startBlocks,
                            viewer->getViewerStats(),0,_numBlocks + 1));
            geode->addDrawable(frameMarkers);

            pos.x() = leftPos;
        }

        // Stats line graph
        {
            pos.y() -= (backgroundSpacing + 2 * backgroundMargin);
            float width = _statsWidth - 4 * backgroundMargin;
            float height = 5 * characterSize;

            // Create a stats graph and add any stats we want to track with it.
            StatsGraph* statsGraph = new StatsGraph(pos,width,height);
            group->addChild(statsGraph);

            for(int i = 0; i < _defaultViewerValueLines.size(); i++)
            {
                if(_defaultViewerValueLines[i]->advanced && !_advanced)
                {
                    continue;
                }
                statsGraph->addStatGraph(viewer->getViewerStats(),
                        viewer->getViewerStats(),
                        _defaultViewerValueLines[i]->color,
                        _defaultViewerValueLines[i]->max,
                        _defaultViewerValueLines[i]->name);
            }

            for(int i = 0; i < _customViewerValueLines.size(); i++)
            {
                if(_customViewerValueLines[i]->advanced && !_advanced)
                {
                    continue;
                }
                statsGraph->addStatGraph(viewer->getViewerStats(),
                        viewer->getViewerStats(),
                        _customViewerValueLines[i]->color,
                        _customViewerValueLines[i]->max,
                        _customViewerValueLines[i]->name);
            }

            if(_statsSubType == ALL_SUB_STATS
                    || _statsSubType == VIEWER_SUB_STATS
                    || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
                    || _statsSubType == NO_CUSTOM_VIEWER_SUB_STATS)
            {
                for(int i = 0; i < _defaultViewerLines.size(); i++)
                {
                    if(_defaultViewerLines[i]->advanced && !_advanced)
                    {
                        continue;
                    }
                    statsGraph->addStatGraph(viewer->getViewerStats(),
                            viewer->getViewerStats(),
                            _defaultViewerLines[i]->color,
                            _defaultViewerLines[i]->max,
                            _defaultViewerLines[i]->name);
                }
            }

            if(_statsSubType == ALL_SUB_STATS
                    || _statsSubType == VIEWER_SUB_STATS)
            {
                for(int i = 0; i < _customViewerLines.size(); i++)
                {
                    if(_customViewerLines[i]->advanced && !_advanced)
                    {
                        continue;
                    }
                    statsGraph->addStatGraph(viewer->getViewerStats(),
                            viewer->getViewerStats(),
                            _customViewerLines[i]->color,
                            _customViewerLines[i]->max,
                            _customViewerLines[i]->name);
                }
            }

            for(osgViewer::ViewerBase::Cameras::iterator citr = cameras.begin();
                    citr != cameras.end(); ++citr)
            {
                for(int i = 0; i < _defaultCameraValueLines.size(); i++)
                {
                    if(_defaultCameraValueLines[i]->advanced && !_advanced)
                    {
                        continue;
                    }
                    statsGraph->addStatGraph(viewer->getViewerStats(),
                            (*citr)->getStats(),
                            _defaultCameraValueLines[i]->color,
                            _defaultCameraValueLines[i]->max,
                            _defaultCameraValueLines[i]->name);
                }

                if(_statsSubType == ALL_SUB_STATS
                        || _statsSubType == CAMERA_SUB_STATS
                        || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
                        || _statsSubType == NO_CUSTOM_CAMERA_SUB_STATS)
                {
                    for(int i = 0; i < _defaultCameraLines.size(); i++)
                    {
                        if(_defaultCameraLines[i]->advanced && !_advanced)
                        {
                            continue;
                        }
                        statsGraph->addStatGraph(viewer->getViewerStats(),
                                (*citr)->getStats(),
                                _defaultCameraLines[i]->color,
                                _defaultCameraLines[i]->max,
                                _defaultCameraLines[i]->name);
                    }
                }

                if(_statsSubType == ALL_SUB_STATS
                        || _statsSubType == CAMERA_SUB_STATS)
                {
                    for(int i = 0; i < _customCameraLines.size(); i++)
                    {
                        if(_customCameraLines[i]->advanced && !_advanced)
                        {
                            continue;
                        }
                        statsGraph->addStatGraph(viewer->getViewerStats(),
                                (*citr)->getStats(),
                                _customCameraLines[i]->color,
                                _customCameraLines[i]->max,
                                _customCameraLines[i]->name);
                    }
                }
            }

            geode->addDrawable(
                    createBackgroundRectangle(
                            pos
                                    + osg::Vec3(-backgroundMargin,
                                            backgroundMargin,0),
                            width + 2 * backgroundMargin,
                            height + 2 * backgroundMargin,backgroundColor));

            pos.x() = leftPos;
            pos.y() -= height + 2 * backgroundMargin;
        }

        // Databasepager stats
        osgViewer::ViewerBase::Scenes scenes;
        viewer->getScenes(scenes);
        for(osgViewer::ViewerBase::Scenes::iterator itr = scenes.begin();
                itr != scenes.end(); ++itr)
        {
            osgViewer::Scene* scene = *itr;
            osgDB::DatabasePager* dp = scene->getDatabasePager();
            if(dp && dp->isRunning())
            {
                pos.y() -= (characterSize + backgroundSpacing);

                geode->addDrawable(
                        createBackgroundRectangle(
                                pos
                                        + osg::Vec3(-backgroundMargin,
                                                characterSize
                                                        + backgroundMargin,0),
                                _statsWidth - 2 * backgroundMargin,
                                characterSize + 2 * backgroundMargin,
                                backgroundColor));

                osg::ref_ptr<osgText::Text> averageLabel = new osgText::Text;
                geode->addDrawable(averageLabel.get());

                averageLabel->setColor(colorDP);
                averageLabel->setFont(font);
                averageLabel->setCharacterSize(characterSize);
                averageLabel->setPosition(pos);
                averageLabel->setText(
                        "DatabasePager time to merge new tiles - average: ");

                pos.x() = averageLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> averageValue = new osgText::Text;
                geode->addDrawable(averageValue.get());

                averageValue->setColor(colorDP);
                averageValue->setFont(font);
                averageValue->setCharacterSize(characterSize);
                averageValue->setPosition(pos);
                averageValue->setText("1000");

                pos.x() = averageValue->getBound().xMax()
                        + 2.0f * characterSize;

                osg::ref_ptr<osgText::Text> minLabel = new osgText::Text;
                geode->addDrawable(minLabel.get());

                minLabel->setColor(colorDP);
                minLabel->setFont(font);
                minLabel->setCharacterSize(characterSize);
                minLabel->setPosition(pos);
                minLabel->setText("min: ");

                pos.x() = minLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> minValue = new osgText::Text;
                geode->addDrawable(minValue.get());

                minValue->setColor(colorDP);
                minValue->setFont(font);
                minValue->setCharacterSize(characterSize);
                minValue->setPosition(pos);
                minValue->setText("1000");

                pos.x() = minValue->getBound().xMax() + 2.0f * characterSize;

                osg::ref_ptr<osgText::Text> maxLabel = new osgText::Text;
                geode->addDrawable(maxLabel.get());

                maxLabel->setColor(colorDP);
                maxLabel->setFont(font);
                maxLabel->setCharacterSize(characterSize);
                maxLabel->setPosition(pos);
                maxLabel->setText("max: ");

                pos.x() = maxLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> maxValue = new osgText::Text;
                geode->addDrawable(maxValue.get());

                maxValue->setColor(colorDP);
                maxValue->setFont(font);
                maxValue->setCharacterSize(characterSize);
                maxValue->setPosition(pos);
                maxValue->setText("1000");

                pos.x() = maxValue->getBound().xMax();

                osg::ref_ptr<osgText::Text> requestsLabel = new osgText::Text;
                geode->addDrawable(requestsLabel.get());

                requestsLabel->setColor(colorDP);
                requestsLabel->setFont(font);
                requestsLabel->setCharacterSize(characterSize);
                requestsLabel->setPosition(pos);
                requestsLabel->setText("requests: ");

                pos.x() = requestsLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> requestList = new osgText::Text;
                geode->addDrawable(requestList.get());

                requestList->setColor(colorDP);
                requestList->setFont(font);
                requestList->setCharacterSize(characterSize);
                requestList->setPosition(pos);
                requestList->setText("0");

                pos.x() = requestList->getBound().xMax() + 2.0f * characterSize;
                ;

                osg::ref_ptr<osgText::Text> compileLabel = new osgText::Text;
                geode->addDrawable(compileLabel.get());

                compileLabel->setColor(colorDP);
                compileLabel->setFont(font);
                compileLabel->setCharacterSize(characterSize);
                compileLabel->setPosition(pos);
                compileLabel->setText("tocompile: ");

                pos.x() = compileLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> compileList = new osgText::Text;
                geode->addDrawable(compileList.get());

                compileList->setColor(colorDP);
                compileList->setFont(font);
                compileList->setCharacterSize(characterSize);
                compileList->setPosition(pos);
                compileList->setText("0");

                pos.x() = maxLabel->getBound().xMax();

                geode->setCullCallback(
                        new PagerCallback(dp,minValue.get(),maxValue.get(),
                                averageValue.get(),requestList.get(),
                                compileList.get(),1000.0));
            }

            pos.x() = leftPos;
        }
    }

    // Camera scene stats
    {
        pos.y() -= (characterSize + backgroundSpacing + 2 * backgroundMargin);

        osg::Group* group = new osg::Group;
        _cameraSceneChildNum = _switch->getNumChildren();
        _switch->addChild(group,false);

        osg::Geode* geode = new osg::Geode();
        geode->setCullingActive(false);
        group->addChild(geode);
        geode->addDrawable(
                createBackgroundRectangle(
                        pos
                                + osg::Vec3(-backgroundMargin,
                                        characterSize + backgroundMargin,0),
                        7 * characterSize + 2 * backgroundMargin,
                        19 * characterSize + 2 * backgroundMargin,
                        backgroundColor));

        // Camera scene & primitive stats static text
        osg::ref_ptr<osgText::Text> camStaticText = new osgText::Text;
        geode->addDrawable(camStaticText.get());
        camStaticText->setColor(staticTextColor);
        camStaticText->setFont(font);
        camStaticText->setCharacterSize(characterSize);
        camStaticText->setPosition(pos);

        std::ostringstream viewStr;
        viewStr.clear();
        viewStr.setf(std::ios::left,std::ios::adjustfield);
        viewStr.width(14);
        viewStr << "Camera" << std::endl;
        viewStr << "" << std::endl; // placeholder for Camera name
        viewStr << "Lights" << std::endl;
        viewStr << "Bins" << std::endl;
        viewStr << "Depth" << std::endl;
        viewStr << "Materials" << std::endl;
        viewStr << "Imposters" << std::endl;
        viewStr << "Drawables" << std::endl;
        viewStr << "Vertices" << std::endl;
        viewStr << "Points" << std::endl;
        viewStr << "Lines" << std::endl;
        viewStr << "Line strips" << std::endl;
        viewStr << "Line loops" << std::endl;
        viewStr << "Triangles" << std::endl;
        viewStr << "Tri. strips" << std::endl;
        viewStr << "Tri. fans" << std::endl;
        viewStr << "Quads" << std::endl;
        viewStr << "Quad strips" << std::endl;
        viewStr << "Polygons" << std::endl;
        viewStr.setf(std::ios::right,std::ios::adjustfield);
        camStaticText->setText(viewStr.str());

        // Move camera block to the right
        pos.x() += 7 * characterSize + 2 * backgroundMargin + backgroundSpacing;

        // Add camera scene stats, one block per camera
        int cameraCounter = 0;
        for(osgViewer::ViewerBase::Cameras::iterator citr = cameras.begin();
                citr != cameras.end(); ++citr)
        {
            geode->addDrawable(
                    createBackgroundRectangle(
                            pos
                                    + osg::Vec3(-backgroundMargin,
                                            characterSize + backgroundMargin,0),
                            5 * characterSize + 2 * backgroundMargin,
                            19 * characterSize + 2 * backgroundMargin,
                            backgroundColor));

            // Camera scene stats
            osg::ref_ptr<osgText::Text> camStatsText = new osgText::Text;
            geode->addDrawable(camStatsText.get());

            camStatsText->setColor(dynamicTextColor);
            camStatsText->setFont(font);
            camStatsText->setCharacterSize(characterSize);
            camStatsText->setPosition(pos);
            camStatsText->setText("");
            camStatsText->setDrawCallback(
                    new CameraSceneStatsTextDrawCallback(*citr,cameraCounter));

            // Move camera block to the right
            pos.x() += 5 * characterSize + 2 * backgroundMargin
                    + backgroundSpacing;
            cameraCounter++;
        }
    }

    // Viewer scene stats
    {
        osg::Group* group = new osg::Group;
        _viewerSceneChildNum = _switch->getNumChildren();
        _switch->addChild(group,false);

        osg::Geode* geode = new osg::Geode();
        geode->setCullingActive(false);
        group->addChild(geode);

        geode->addDrawable(
                createBackgroundRectangle(
                        pos
                                + osg::Vec3(-backgroundMargin,
                                        characterSize + backgroundMargin,0),
                        6 * characterSize + 2 * backgroundMargin,
                        12 * characterSize + 2 * backgroundMargin,
                        backgroundColor));

        // View scene stats static text
        osg::ref_ptr<osgText::Text> camStaticText = new osgText::Text;
        geode->addDrawable(camStaticText.get());
        camStaticText->setColor(staticTextColor);
        camStaticText->setFont(font);
        camStaticText->setCharacterSize(characterSize);
        camStaticText->setPosition(pos);

        std::ostringstream viewStr;
        viewStr.clear();
        viewStr.setf(std::ios::left,std::ios::adjustfield);
        viewStr.width(14);
        viewStr << "View" << std::endl;
        viewStr << " " << std::endl;
        viewStr << "Stateset" << std::endl;
        viewStr << "Group" << std::endl;
        viewStr << "Transform" << std::endl;
        viewStr << "LOD" << std::endl;
        viewStr << "Switch" << std::endl;
        viewStr << "Geode" << std::endl;
        viewStr << "Drawable" << std::endl;
        viewStr << "Geometry" << std::endl;
        viewStr << "Vertices" << std::endl;
        viewStr << "Primitives" << std::endl;
        viewStr.setf(std::ios::right,std::ios::adjustfield);
        camStaticText->setText(viewStr.str());

        // Move viewer block to the right
        pos.x() += 6 * characterSize + 2 * backgroundMargin + backgroundSpacing;

        std::vector<osgViewer::View*> views;
        viewer->getViews(views);

        std::vector<osgViewer::View*>::iterator it;
        int viewCounter = 0;
        for(it = views.begin(); it != views.end(); ++it)
        {
            geode->addDrawable(
                    createBackgroundRectangle(
                            pos
                                    + osg::Vec3(-backgroundMargin,
                                            characterSize + backgroundMargin,0),
                            10 * characterSize + 2 * backgroundMargin,
                            12 * characterSize + 2 * backgroundMargin,
                            backgroundColor));

            // Text for scene statistics
            osgText::Text* text = new osgText::Text;
            geode->addDrawable(text);

            text->setColor(dynamicTextColor);
            text->setFont(font);
            text->setCharacterSize(characterSize);
            text->setPosition(pos);
            text->setDrawCallback(
                    new ViewSceneStatsTextDrawCallback(*it,viewCounter));

            pos.x() += 10 * characterSize + 2 * backgroundMargin
                    + backgroundSpacing;
            viewCounter++;
        }
    }
}

void CVRStatsHandler::setCollect(osgViewer::ViewerBase * viewer)
{
    for(std::map<std::string,bool>::iterator it = _collectMapViewer.begin();
            it != _collectMapViewer.end(); it++)
    {
        viewer->getViewerStats()->collectStats(it->first,false);
    }

    osgViewer::ViewerBase::Cameras cameras;
    viewer->getCameras(cameras);

    for(std::map<std::string,bool>::iterator it = _collectMapCameras.begin();
            it != _collectMapCameras.end(); it++)
    {
        for(osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
                itr != cameras.end(); ++itr)
        {
            osg::Stats* stats = (*itr)->getStats();
            if(stats)
            {
                stats->collectStats(it->first,false);
            }
        }
    }

    _collectMapViewer.clear();
    _collectMapCameras.clear();

    if(_statsType >= FRAME_RATE)
    {
        for(int i = 0; i < _defaultViewerValues.size(); i++)
        {
            if(_defaultViewerValues[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapViewer[_defaultViewerValues[i]->collectName] = true;
        }

	for(int i = 0; i < _customViewerValues.size(); i++)
        {
            if(_customViewerValues[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapViewer[_customViewerValues[i]->collectName] = true;
        }

        for(int i = 0; i < _defaultCameraValues.size(); i++)
        {
            if(_defaultCameraValues[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapCameras[_defaultCameraValues[i]->collectName] = true;
        }

	for(int i = 0; i < _customCameraValues.size(); i++)
        {
            if(_customCameraValues[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapCameras[_customCameraValues[i]->collectName] = true;
        }
    }

    if(_statsType >= VIEWER_STATS)
    {
        for(int i = 0; i < _defaultViewerValueLines.size(); i++)
        {
            if(_defaultViewerValueLines[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapViewer[_defaultViewerValueLines[i]->collectName] = true;
        }

        for(int i = 0; i < _defaultCameraValueLines.size(); i++)
        {
            if(_defaultCameraValueLines[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapCameras[_defaultCameraValueLines[i]->collectName] = true;
        }

        for(int i = 0; i < _customViewerValueLines.size(); i++)
        {
            if(_customViewerValueLines[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapViewer[_customViewerValueLines[i]->collectName] = true;
        }

        for(int i = 0; i < _customCameraValueLines.size(); i++)
        {
            if(_customCameraValueLines[i]->advanced && !_advanced)
            {
                continue;
            }
            _collectMapCameras[_customCameraValueLines[i]->collectName] = true;
        }

        if(_statsSubType == PLUGINS_SUB_STATS
                || _statsSubType == NO_CUSTOM_PLUGINS_SUB_STATS)
        {
            _collectMapViewer["CalVRStatsPlugins"] = true;
        }

        if(_statsSubType == ALL_SUB_STATS || _statsSubType == VIEWER_SUB_STATS
                || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
                || _statsSubType == NO_CUSTOM_VIEWER_SUB_STATS)
        {
            for(int i = 0; i < _defaultViewerTimeBars.size(); i++)
            {
                if(_defaultViewerTimeBars[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapViewer[_defaultViewerTimeBars[i]->collectName] =
                        true;
            }
            for(int i = 0; i < _defaultViewerLines.size(); i++)
            {
                if(_defaultViewerLines[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapViewer[_defaultViewerLines[i]->collectName] = true;
            }
        }

        if(_statsSubType == ALL_SUB_STATS || _statsSubType == VIEWER_SUB_STATS)
        {
            for(int i = 0; i < _customViewerTimeBars.size(); i++)
            {
                if(_customViewerTimeBars[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapViewer[_customViewerTimeBars[i]->collectName] = true;
            }
            for(int i = 0; i < _customViewerLines.size(); i++)
            {
                if(_customViewerLines[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapViewer[_customViewerLines[i]->collectName] = true;
            }
        }

        if(_statsSubType == ALL_SUB_STATS || _statsSubType == CAMERA_SUB_STATS
                || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
                || _statsSubType == NO_CUSTOM_CAMERA_SUB_STATS)
        {
            for(int i = 0; i < _defaultCameraTimeBars.size(); i++)
            {
                if(_defaultCameraTimeBars[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapCameras[_defaultCameraTimeBars[i]->collectName] =
                        true;
            }
            for(int i = 0; i < _defaultCameraLines.size(); i++)
            {
                if(_defaultCameraLines[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapCameras[_defaultCameraLines[i]->collectName] = true;
            }
        }

        if(_statsSubType == ALL_SUB_STATS || _statsSubType == CAMERA_SUB_STATS)
        {
            for(int i = 0; i < _customCameraTimeBars.size(); i++)
            {
                if(_customCameraTimeBars[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapCameras[_customCameraTimeBars[i]->collectName] =
                        true;
            }
            for(int i = 0; i < _customCameraLines.size(); i++)
            {
                if(_customCameraLines[i]->advanced && !_advanced)
                {
                    continue;
                }
                _collectMapCameras[_customCameraLines[i]->collectName] = true;
            }
        }
    }

    for(std::map<std::string,bool>::iterator it = _collectMapViewer.begin();
            it != _collectMapViewer.end(); it++)
    {
        viewer->getViewerStats()->collectStats(it->first,true);
    }

    for(std::map<std::string,bool>::iterator it = _collectMapCameras.begin();
            it != _collectMapCameras.end(); it++)
    {
        for(osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
                itr != cameras.end(); ++itr)
        {
            osg::Stats* stats = (*itr)->getStats();
            if(stats)
            {
                stats->collectStats(it->first,true);
            }
        }
    }
}

void CVRStatsHandler::calculateStartBlocks(float & startBlocks, float leftPos,
        float characterSize, std::string & font)
{
    osg::ref_ptr<osgText::Text> label = new osgText::Text();
    label->setFont(font);
    label->setCharacterSize(characterSize);
    label->setPosition(osg::Vec3(leftPos,0.0,0.0));
    label->setText("");

    if(_statsSubType == PLUGINS_SUB_STATS
            || _statsSubType == NO_CUSTOM_PLUGINS_SUB_STATS)
    {
        std::vector<std::string> pluginList =
                PluginManager::instance()->getLoadedPluginList();
        for(int i = 0; i < pluginList.size(); i++)
        {
            //extra space added to pad side
            label->setText(pluginList[i] + ":  000.00");
            if(label->getBound().xMax() > startBlocks)
            {
                startBlocks = label->getBound().xMax();
            }
        }
    }

    if(_statsSubType == ALL_SUB_STATS || _statsSubType == VIEWER_SUB_STATS
            || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
            || _statsSubType == NO_CUSTOM_VIEWER_SUB_STATS)
    {
        for(int i = 0; i < _defaultViewerTimeBars.size(); i++)
        {
            if(_defaultViewerTimeBars[i]->advanced && !_advanced)
            {
                continue;
            }
            label->setText(_defaultViewerTimeBars[i]->label + "  000.00");
            if(label->getBound().xMax() > startBlocks)
            {
                startBlocks = label->getBound().xMax();
            }
        }
    }

    if(_statsSubType == ALL_SUB_STATS || _statsSubType == VIEWER_SUB_STATS)
    {
        for(int i = 0; i < _customViewerTimeBars.size(); i++)
        {
            if(_customViewerTimeBars[i]->advanced && !_advanced)
            {
                continue;
            }
            label->setText(_customViewerTimeBars[i]->label + "  000.00");
            if(label->getBound().xMax() > startBlocks)
            {
                startBlocks = label->getBound().xMax();
            }
        }
    }

    if(_statsSubType == ALL_SUB_STATS || _statsSubType == CAMERA_SUB_STATS
            || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
            || _statsSubType == NO_CUSTOM_CAMERA_SUB_STATS)
    {
        for(int i = 0; i < _defaultCameraTimeBars.size(); i++)
        {
            if(_defaultCameraTimeBars[i]->advanced && !_advanced)
            {
                continue;
            }
            label->setText(_defaultCameraTimeBars[i]->label + "  000.00");
            if(label->getBound().xMax() > startBlocks)
            {
                startBlocks = label->getBound().xMax();
            }
        }
    }

    if(_statsSubType == ALL_SUB_STATS || _statsSubType == CAMERA_SUB_STATS)
    {
        for(int i = 0; i < _customCameraTimeBars.size(); i++)
        {
            if(_customCameraTimeBars[i]->advanced && !_advanced)
            {
                continue;
            }
            label->setText(_customCameraTimeBars[i]->label + "  000.00");
            if(label->getBound().xMax() > startBlocks)
            {
                startBlocks = label->getBound().xMax();
            }
        }
    }
}

void CVRStatsHandler::createTimeBar(osg::Stats * viewerStats, osg::Stats* stats,
        osg::Geode * geode, osg::Vec3 & pos, StatTimeBarInfo * stb,
        std::string & font, float characterSize, float startBlocks,
        float leftPos)
{
    pos.x() = leftPos;

    osg::ref_ptr<osgText::Text> label = new osgText::Text;
    geode->addDrawable(label.get());

    label->setColor(stb->color);
    label->setFont(font);
    label->setCharacterSize(characterSize);
    label->setPosition(pos);
    label->setText(stb->label);

    pos.x() = label->getBound().xMax() + _spaceSize;

    osg::ref_ptr<osgText::Text> value = new osgText::Text;
    geode->addDrawable(value.get());

    value->setColor(stb->color);
    value->setFont(font);
    value->setCharacterSize(characterSize);
    value->setPosition(pos);
    value->setText("0.0");

    value->setDrawCallback(
            new AveragedValueTextDrawCallback(stats ? stats : viewerStats,
                    stb->nameDuration,-1,false,1000.0));

    pos.x() = startBlocks;
    osg::Geometry* geometry = createGeometry(pos,characterSize * 0.8,
            stb->colorAlpha,_numBlocks);
    geometry->setDrawCallback(
            new BlockDrawCallback(this,startBlocks,viewerStats,
                    stats ? stats : viewerStats,stb->nameTimeStart,
                    stb->nameTimeEnd,-1,_numBlocks));
    geode->addDrawable(geometry);

    pos.y() -= characterSize * 1.5f;
}

osg::Node* CVRStatsHandler::createCameraTimeStats(std::string& font,
        osg::Vec3& pos, float startBlocks, bool acquireGPUStats,
        float characterSize, osg::Stats* viewerStats, osg::Camera* camera)
{
    osg::Stats* stats = camera->getStats();
    if(!stats)
        return 0;

    osg::Group* group = new osg::Group;

    osg::Geode* geode = new osg::Geode();
    group->addChild(geode);

    float leftPos = pos.x();

    if(_statsSubType == ALL_SUB_STATS || _statsSubType == CAMERA_SUB_STATS
            || _statsSubType == NO_CUSTOM_ALL_SUB_STATS
            || _statsSubType == NO_CUSTOM_CAMERA_SUB_STATS)
    {
        for(int i = 0; i < _defaultCameraTimeBars.size(); i++)
        {
            if(_defaultCameraTimeBars[i]->advanced && !_advanced)
            {
                continue;
            }
            createTimeBar(viewerStats,stats,geode,pos,_defaultCameraTimeBars[i],
                    font,characterSize,startBlocks,leftPos);
        }
    }

    if(_statsSubType == ALL_SUB_STATS || _statsSubType == CAMERA_SUB_STATS)
    {
        for(int i = 0; i < _customCameraTimeBars.size(); i++)
        {
            if(_customCameraTimeBars[i]->advanced && !_advanced)
            {
                continue;
            }
            createTimeBar(viewerStats,stats,geode,pos,_customCameraTimeBars[i],
                    font,characterSize,startBlocks,leftPos);
        }
    }

    pos.x() = leftPos;

    return group;
}

void CVRStatsHandler::refresh()
{
    if(!_initialized)
    {
        return;
    }

    if(_switch)
    {
        _camera->removeChild(_switch);
        setUpScene(_viewer);
    }

    switch(_statsType)
    {
        case VIEWER_SCENE_STATS:
            _switch->setValue(_viewerSceneChildNum,true);
        case CAMERA_SCENE_STATS:
            _switch->setValue(_cameraSceneChildNum,true);
        case VIEWER_STATS:
            _switch->setValue(_viewerChildNum,true);
        case FRAME_RATE:
            _switch->setValue(_viewerValuesChildNum,true);
        case NO_STATS:
            break;
        default:
            break;
    }

    setCollect(_viewer);
}

void CVRStatsHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("S","On screen stats.");
    usage.addKeyboardMouseBinding("P","Output stats to console.");
}

void CVRStatsHandler::addStatValue(StatAddType addType, std::string label,
        std::string statName, osg::Vec3 color, std::string collectName,
        bool average, bool advanced)
{
    StatValueInfo * svi = new StatValueInfo;
    svi->label = label;
    svi->name = statName;
    svi->average = average;
    svi->collectName = collectName;
    svi->advanced = advanced;
    svi->color.x() = color.x();
    svi->color.y() = color.y();
    svi->color.z() = color.z();
    svi->color.a() = 1.0;
    svi->colorAlpha = svi->color;
    svi->colorAlpha.a() = 0.5;

    if(addType == VIEWER_STAT)
    {
        _customViewerValues.push_back(svi);
    }
    else
    {
        _customCameraValues.push_back(svi);
    }

    refresh();
}

void CVRStatsHandler::addStatValueWithLine(StatAddType addType,
        std::string label, std::string statName, osg::Vec3 color,
        std::string collectName, float lineMax, bool average, bool advanced)
{
    StatValueInfo * svi = new StatValueInfo;
    svi->label = label;
    svi->name = statName;
    svi->average = average;
    svi->collectName = collectName;
    svi->advanced = advanced;
    svi->color.x() = color.x();
    svi->color.y() = color.y();
    svi->color.z() = color.z();
    svi->color.a() = 1.0;
    svi->colorAlpha = svi->color;
    svi->colorAlpha.a() = 0.5;

    StatLineInfo * sli = new StatLineInfo;
    sli->name = statName;
    sli->max = lineMax;
    sli->collectName = collectName;
    sli->advanced = advanced;
    sli->color = svi->color;
    sli->colorAlpha = svi->colorAlpha;

    if(addType == VIEWER_STAT)
    {
        _customViewerValues.push_back(svi);
        _customViewerValueLines.push_back(sli);
    }
    else
    {
        _customCameraValues.push_back(svi);
        _customCameraValueLines.push_back(sli);
    }

    refresh();
}

void CVRStatsHandler::addStatTimeBar(StatAddType addType, std::string label,
        std::string statDurationName, std::string statStartTimeName,
        std::string statEndTimeName, osg::Vec3 color, std::string collectName,
        bool advanced)
{
    StatTimeBarInfo * sbi = new StatTimeBarInfo;
    sbi->label = label;
    sbi->nameDuration = statDurationName;
    sbi->nameTimeStart = statStartTimeName;
    sbi->nameTimeEnd = statEndTimeName;
    sbi->collectName = collectName;
    sbi->advanced = advanced;
    sbi->color.x() = color.x();
    sbi->color.y() = color.y();
    sbi->color.z() = color.z();
    sbi->color.a() = 1.0;
    sbi->colorAlpha = sbi->color;
    sbi->colorAlpha.a() = 0.5;

    if(addType == VIEWER_STAT)
    {
        _customViewerTimeBars.push_back(sbi);
    }
    else
    {
        _customCameraTimeBars.push_back(sbi);
    }

    refresh();
}

void CVRStatsHandler::addStatTimeBarWithLine(StatAddType addType,
        std::string label, std::string statDurationName,
        std::string statStartTimeName, std::string statEndTimeName,
        osg::Vec3 color, std::string collectName, float lineMax, bool advanced)
{
    StatTimeBarInfo * sbi = new StatTimeBarInfo;
    sbi->label = label;
    sbi->nameDuration = statDurationName;
    sbi->nameTimeStart = statStartTimeName;
    sbi->nameTimeEnd = statEndTimeName;
    sbi->collectName = collectName;
    sbi->advanced = advanced;
    sbi->color.x() = color.x();
    sbi->color.y() = color.y();
    sbi->color.z() = color.z();
    sbi->color.a() = 1.0;
    sbi->colorAlpha = sbi->color;
    sbi->colorAlpha.a() = 0.5;

    StatLineInfo * sli = new StatLineInfo;
    sli->name = statDurationName;
    sli->max = lineMax;
    sli->collectName = collectName;
    sli->advanced = advanced;
    sli->color = sbi->color;
    sli->colorAlpha = sbi->colorAlpha;

    if(addType == VIEWER_STAT)
    {
        _customViewerTimeBars.push_back(sbi);
        _customViewerLines.push_back(sli);
    }
    else
    {
        _customCameraTimeBars.push_back(sbi);
        _customCameraLines.push_back(sli);
    }

    refresh();
}

void CVRStatsHandler::addStatLine(StatAddType addType, std::string statName,
        osg::Vec3 color, std::string collectName, float lineMax, bool advanced)
{
    StatLineInfo * sli = new StatLineInfo;
    sli->name = statName;
    sli->max = lineMax;
    sli->collectName = collectName;
    sli->advanced = advanced;
    sli->color.x() = color.x();
    sli->color.y() = color.y();
    sli->color.z() = color.z();
    sli->color.a() = 1.0;
    sli->colorAlpha = sli->color;
    sli->colorAlpha.a() = 0.5;

    if(addType == VIEWER_STAT)
    {
        _customViewerLines.push_back(sli);
    }
    else
    {
        _customCameraLines.push_back(sli);
    }

    refresh();
}

void CVRStatsHandler::removeStatValue(std::string statName)
{
    bool statRemoved = false;

    for(std::vector<StatValueInfo*>::iterator it = _customViewerValues.begin();
            it != _customViewerValues.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customViewerValues.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatValueInfo*>::iterator it = _customCameraValues.begin();
            it != _customCameraValues.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customCameraValues.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    if(statRemoved)
    {
        refresh();
    }
}

void CVRStatsHandler::removeStatValueWithLine(std::string statName)
{
    bool statRemoved = false;

    for(std::vector<StatValueInfo*>::iterator it = _customViewerValues.begin();
            it != _customViewerValues.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customViewerValues.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatValueInfo*>::iterator it = _customCameraValues.begin();
            it != _customCameraValues.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customCameraValues.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it =
            _customViewerValueLines.begin();
            it != _customViewerValueLines.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customViewerValueLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it =
            _customCameraValueLines.begin();
            it != _customCameraValueLines.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customCameraValueLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    if(statRemoved)
    {
        refresh();
    }
}

void CVRStatsHandler::removeStatTimeBar(std::string statDurationName)
{
    bool statRemoved = false;

    for(std::vector<StatTimeBarInfo*>::iterator it =
            _customViewerTimeBars.begin(); it != _customViewerTimeBars.end();)
    {
        if((*it)->nameDuration == statDurationName)
        {
            delete (*it);
            it = _customViewerTimeBars.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatTimeBarInfo*>::iterator it =
            _customCameraTimeBars.begin(); it != _customCameraTimeBars.end();)
    {
        if((*it)->nameDuration == statDurationName)
        {
            delete (*it);
            it = _customCameraTimeBars.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    if(statRemoved)
    {
        refresh();
    }
}

void CVRStatsHandler::removeStatTimeBarWithLine(std::string statDurationName)
{
    bool statRemoved = false;

    for(std::vector<StatTimeBarInfo*>::iterator it =
            _customViewerTimeBars.begin(); it != _customViewerTimeBars.end();)
    {
        if((*it)->nameDuration == statDurationName)
        {
            delete (*it);
            it = _customViewerTimeBars.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatTimeBarInfo*>::iterator it =
            _customCameraTimeBars.begin(); it != _customCameraTimeBars.end();)
    {
        if((*it)->nameDuration == statDurationName)
        {
            delete (*it);
            it = _customCameraTimeBars.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it = _customViewerLines.begin();
            it != _customViewerLines.end();)
    {
        if((*it)->name == statDurationName)
        {
            delete (*it);
            it = _customViewerLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it = _customCameraLines.begin();
            it != _customCameraLines.end();)
    {
        if((*it)->name == statDurationName)
        {
            delete (*it);
            it = _customCameraLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    if(statRemoved)
    {
        refresh();
    }
}

void CVRStatsHandler::removeStatLine(std::string statName)
{
    bool statRemoved = false;

    for(std::vector<StatLineInfo*>::iterator it =
            _customViewerValueLines.begin();
            it != _customViewerValueLines.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customViewerValueLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it =
            _customCameraValueLines.begin();
            it != _customCameraValueLines.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customCameraValueLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it = _customViewerLines.begin();
            it != _customViewerLines.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customViewerLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    for(std::vector<StatLineInfo*>::iterator it = _customCameraLines.begin();
            it != _customCameraLines.end();)
    {
        if((*it)->name == statName)
        {
            delete (*it);
            it = _customCameraLines.erase(it);
            statRemoved = true;
        }
        else
        {
            it++;
        }
    }

    if(statRemoved)
    {
        refresh();
    }
}
