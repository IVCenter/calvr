/**
 * @file ScreenLenticular.h
 */

#ifndef CALVR_SCREEN_LENTICULAR_H
#define CALVR_SCREEN_LENTICULAR_H

#include <coInterleaver.h>

#include <kernel/ScreenBase.h>

#include <osg/Camera>
#include <OpenThreads/Mutex>

#include <vector>
#include <string>
#include <map>

namespace cvr
{

/**
 * @brief creates a screen to render lenticular stereo
 */
class ScreenLenticular : public ScreenBase
{
    public:
        ScreenLenticular();
        virtual ~ScreenLenticular();

        struct PreDrawCallback : public osg::Camera::Camera::DrawCallback
        {
                virtual void operator()(osg::RenderInfo & ri) const;
                il_display * lentDisp;
                int eyes;
                int eye;
                std::string vertShader;
                std::string fragShader;
                std::map<int,il_context*> * contextMap;
                static OpenThreads::Mutex mutex;
                osg::Camera * cam;
        };

        struct PostDrawCallback : public osg::Camera::Camera::DrawCallback
        {
                virtual void operator()(osg::RenderInfo & ri) const;
                mutable int count;
                int eyes;
                std::map<int,il_context*> * contextMap;
                il_display * lentDisp;
                float * eyePos;
                std::vector<osg::ref_ptr<osg::Camera> > * cameraList;
        };

        virtual void init(int mode = 0);
        virtual void computeViewProj();
        virtual void updateCamera();
        virtual void setClearColor(osg::Vec4 color);
        virtual ScreenInfo * findScreenInfo(osg::Camera * c);
    protected:
        void initDisplay();
        bool readFromThumbFile(std::string file);

        int _eyes;
        std::vector<osg::ref_ptr<osg::Camera> > _cameraList;
        std::vector<osg::Matrix> _viewList;
        std::vector<osg::Matrix> _projList;

        osg::Quat _invScreenRotation;

        std::string _shaderDir;

        float _ipd;
        float * _eyePos;
        float _viewDistance;

        int _screenIndex;

        il_display * _lentDisp;
        static std::map<int,il_context*> _contextMap;

        std::vector<PreDrawCallback*> _preCallbackList;
        PostDrawCallback* _postCallback;
};

}

#endif
