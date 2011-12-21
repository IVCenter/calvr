#include <kernel/MultiViewerCullVisitor.h>
#include <osg/Geode>
#include <osg/OcclusionQueryNode>
#include <osg/Billboard>
#include <osg/LightSource>
#include <osg/ClipNode>
#include <osg/TexGenNode>
#include <osg/Projection>
#include <osg/LOD>
#include <osg/OccluderNode>
#include <osg/Version>
#include <osg/Matrix>

using namespace cvr;
using namespace osgUtil;
using namespace osg;

MultiViewerCullVisitor::MultiViewerCullVisitor() :
        CullVisitor()
{
    //std::cerr << "My cull visitor created." << std::endl;
    _cullingStatus = true;
    _firstCullStatus = true;
    _skipCull = false;
}

MultiViewerCullVisitor::MultiViewerCullVisitor(const MultiViewerCullVisitor& cv) :
        CullVisitor(cv)
{
    _cullingStatus = cv._cullingStatus;
}

// osgUtil::CullVisitor functions
// I wish there was an easier way, but isCulled is not virtual

inline CullVisitor::value_type distance(const osg::Vec3& coord,
        const osg::Matrix& matrix)
{
    return -((CullVisitor::value_type)coord[0]
            * (CullVisitor::value_type)matrix(0,2)
            + (CullVisitor::value_type)coord[1]
                    * (CullVisitor::value_type)matrix(1,2)
            + (CullVisitor::value_type)coord[2]
                    * (CullVisitor::value_type)matrix(2,2) + matrix(3,2));
}

void MultiViewerCullVisitor::apply(Node& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;
    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(Geode& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;
    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    // traverse any call callbacks and traverse any children.
    handle_cull_callbacks_and_traverse(node);

    RefMatrix& matrix = *getModelViewMatrix();
    for(unsigned int i = 0; i < node.getNumDrawables(); ++i)
    {
        Drawable* drawable = node.getDrawable(i);
        const BoundingBox &bb = drawable->getBound();

        if(drawable->getCullCallback())
        {
            if(drawable->getCullCallback()->cull(this,drawable,&_renderInfo)
                    == true)
                continue;
        }

        //else
        {
            if(node.isCullingActive() && isCulled(bb))
                continue;
        }

        if(_computeNearFar && bb.valid())
        {
            if(!updateCalculatedNearFar(matrix,*drawable,false))
                continue;
        }

        // need to track how push/pops there are, so we can unravel the stack correctly.
        unsigned int numPopStateSetRequired = 0;

        // push the geoset's state on the geostate stack.    
        StateSet* stateset = drawable->getStateSet();
        if(stateset)
        {
            ++numPopStateSetRequired;
            pushStateSet(stateset);
        }

        CullingSet& cs = getCurrentCullingSet();
        if(!cs.getStateFrustumList().empty())
        {
            osg::CullingSet::StateFrustumList& sfl = cs.getStateFrustumList();
            for(osg::CullingSet::StateFrustumList::iterator itr = sfl.begin();
                    itr != sfl.end(); ++itr)
            {
                if(itr->second.contains(bb))
                {
                    ++numPopStateSetRequired;
                    pushStateSet(itr->first.get());
                }
            }
        }

        float depth = bb.valid() ? distance(bb.center(),matrix) : 0.0f;

        if(osg::isNaN(depth))
        {
            /*OSG_NOTIFY(osg::NOTICE)<<"CullVisitor::apply(Geode&) detected NaN,"<<std::endl
             <<"    depth="<<depth<<", center=("<<bb.center()<<"),"<<std::endl
             <<"    matrix="<<matrix<<std::endl;
             OSG_NOTIFY(osg::DEBUG_INFO) << "    NodePath:" << std::endl;
             for (NodePath::const_iterator i = getNodePath().begin(); i != getNodePath().end(); ++i)
             {
             OSG_NOTIFY(osg::DEBUG_INFO) << "        \"" << (*i)->getName() << "\"" << std::endl;
             }*/
        }
        else
        {
            addDrawableAndDepth(drawable,&matrix,depth);
        }

        for(unsigned int i = 0; i < numPopStateSetRequired; ++i)
        {
            popStateSet();
        }

    }

    // pop the node's state off the geostate stack.    
    if(node_state)
        popStateSet();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(Billboard& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;
    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    // traverse any call callbacks and traverse any children.
    handle_cull_callbacks_and_traverse(node);

    const Vec3& eye_local = getEyeLocal();
    const RefMatrix& modelview = *getModelViewMatrix();

    for(unsigned int i = 0; i < node.getNumDrawables(); ++i)
    {
        const Vec3& pos = node.getPosition(i);

        Drawable* drawable = node.getDrawable(i);
        // need to modify isCulled to handle the billboard offset.
        // if (isCulled(drawable->getBound())) continue;

        if(drawable->getCullCallback())
        {
            if(drawable->getCullCallback()->cull(this,drawable,&_renderInfo)
                    == true)
                continue;
        }

        RefMatrix* billboard_matrix = createOrReuseMatrix(modelview);

        node.computeMatrix(*billboard_matrix,eye_local,pos);

        if(_computeNearFar && drawable->getBound().valid())
            updateCalculatedNearFar(*billboard_matrix,*drawable,true);
        float depth = distance(pos,modelview);
        /*
         if (_computeNearFar)
         {
         if (d<_computed_znear)
         {
         if (d<0.0) OSG_NOTIFY(osg::WARN)<<"Alerting billboard handling ="<<d<< std::endl;
         _computed_znear = d;
         }
         if (d>_computed_zfar) _computed_zfar = d;
         }
         */
        StateSet* stateset = drawable->getStateSet();
        if(stateset)
            pushStateSet(stateset);

        if(osg::isNaN(depth))
        {
            /*OSG_NOTIFY(osg::NOTICE)<<"CullVisitor::apply(Billboard&) detected NaN,"<<std::endl
             <<"    depth="<<depth<<", pos=("<<pos<<"),"<<std::endl
             <<"    *billboard_matrix="<<*billboard_matrix<<std::endl;
             OSG_NOTIFY(osg::DEBUG_INFO) << "    NodePath:" << std::endl;
             for (NodePath::const_iterator i = getNodePath().begin(); i != getNodePath().end(); ++i)
             {
             OSG_NOTIFY(osg::DEBUG_INFO) << "        \"" << (*i)->getName() << "\"" << std::endl;
             }*/
        }
        else
        {
            addDrawableAndDepth(drawable,billboard_matrix,depth);
        }

        if(stateset)
            popStateSet();

    }

    // pop the node's state off the geostate stack.    
    if(node_state)
        popStateSet();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(LightSource& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    StateAttribute* light = node.getLight();
    if(light)
    {
        if(node.getReferenceFrame() == osg::LightSource::RELATIVE_RF)
        {
            RefMatrix& matrix = *getModelViewMatrix();
            addPositionedAttribute(&matrix,light);
        }
        else
        {
            // relative to absolute.
            addPositionedAttribute(0,light);
        }
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.    
    if(node_state)
        popStateSet();
}

void MultiViewerCullVisitor::apply(ClipNode& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    RefMatrix& matrix = *getModelViewMatrix();

    const ClipNode::ClipPlaneList& planes = node.getClipPlaneList();
    for(ClipNode::ClipPlaneList::const_iterator itr = planes.begin();
            itr != planes.end(); ++itr)
    {
        if(node.getReferenceFrame() == osg::ClipNode::RELATIVE_RF)
        {
            addPositionedAttribute(&matrix,itr->get());
        }
        else
        {
            addPositionedAttribute(0,itr->get());
        }
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.    
    if(node_state)
        popStateSet();
}

void MultiViewerCullVisitor::apply(TexGenNode& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    if(node.getReferenceFrame() == osg::TexGenNode::RELATIVE_RF)
    {
        RefMatrix& matrix = *getModelViewMatrix();
        addPositionedTextureAttribute(node.getTextureUnit(),&matrix,
                node.getTexGen());
    }
    else
    {
        addPositionedTextureAttribute(node.getTextureUnit(),0,node.getTexGen());
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.    
    if(node_state)
        popStateSet();
}

void MultiViewerCullVisitor::apply(Group& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;

    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(Transform& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;

    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    ref_ptr < RefMatrix > matrix = createOrReuseMatrix(*getModelViewMatrix());
    node.computeLocalToWorldMatrix(*matrix,this);
    pushModelViewMatrix(matrix.get(),node.getReferenceFrame());

    handle_cull_callbacks_and_traverse(node);

    popModelViewMatrix();

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(Projection& node)
{

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    // record previous near and far values.
    float previous_znear = _computed_znear;
    float previous_zfar = _computed_zfar;

    // take a copy of the current near plane candidates
    DistanceMatrixDrawableMap previousNearPlaneCandidateMap;
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;

    ref_ptr < RefMatrix > matrix = createOrReuseMatrix(node.getMatrix());
    pushProjectionMatrix(matrix.get());

    //OSG_NOTIFY(osg::INFO)<<"Push projection "<<*matrix<<std::endl;

    // note do culling check after the frustum has been updated to ensure
    // that the node is not culled prematurely.

    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;

    if(!isCulled(node))
    {
        handle_cull_callbacks_and_traverse(node);
    }

    _firstCullStatus = firstStatus;
    _cullingStatus = status;

    popProjectionMatrix();

    //OSG_NOTIFY(osg::INFO)<<"Pop projection "<<*matrix<<std::endl;

    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;

    // swap back the near plane candidates
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void MultiViewerCullVisitor::apply(Switch& node)
{
    apply((Group&)node);
}

void MultiViewerCullVisitor::apply(LOD& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;

    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(osg::ClearNode& node)
{
    // simply override the current earth sky.
    if(node.getRequiresClear())
    {
        getCurrentRenderBin()->getStage()->setClearColor(node.getClearColor());
        getCurrentRenderBin()->getStage()->setClearMask(node.getClearMask());
    }
    else
    {
        // we have an earth sky implementation to do the work for us
        // so we don't need to clear.
        getCurrentRenderBin()->getStage()->setClearMask(0);
    }

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

}

namespace osgUtil
{

class RenderStageCache : public osg::Object
{
    public:

        RenderStageCache()
        {
        }
        RenderStageCache(const RenderStageCache&, const osg::CopyOp&)
        {
        }

        META_Object(osgUtil, RenderStageCache);

        void setRenderStage(CullVisitor* cv, RenderStage* rs)
        {
            OpenThreads::ScopedLock < OpenThreads::Mutex > lock(_mutex);
            _renderStageMap[cv] = rs;
        }

        RenderStage* getRenderStage(osgUtil::CullVisitor* cv)
        {
            OpenThreads::ScopedLock < OpenThreads::Mutex > lock(_mutex);
            return _renderStageMap[cv].get();
        }

        typedef std::map<CullVisitor*,osg::ref_ptr<RenderStage> > RenderStageMap;

        OpenThreads::Mutex _mutex;
        RenderStageMap _renderStageMap;
};

}

void MultiViewerCullVisitor::apply(osg::Camera& camera)
{
    //std::cerr << "MVCV camera" << std::endl;
    // push the node's state.
    StateSet* node_state = camera.getStateSet();
    if(node_state)
        pushStateSet(node_state);

//#define DEBUG_CULLSETTINGS

#ifdef DEBUG_CULLSETTINGS
    if (osg::isNotifyEnabled(osg::NOTICE))
    {
        notify(osg::NOTICE)<<std::endl<<std::endl<<"CullVisitor, before : ";
        write(osg::notify(osg::NOTICE));
    }
#endif

    // Save current cull settings
    CullSettings saved_cull_settings(*this);

#ifdef DEBUG_CULLSETTINGS
    if (osg::isNotifyEnabled(osg::NOTICE))
    {
        osg::notify(osg::NOTICE)<<"CullVisitor, saved_cull_settings : ";
        saved_cull_settings.write(osg::notify(osg::NOTICE));
    }
#endif

#if 1
    // set cull settings from this Camera
    setCullSettings(camera);

#ifdef DEBUG_CULLSETTINGS
    osg::notify(osg::NOTICE)<<"CullVisitor, after setCullSettings(camera) : ";
    write(osg::notify(osg::NOTICE));
#endif
    // inherit the settings from above
    inheritCullSettings(saved_cull_settings,camera.getInheritanceMask());

#ifdef DEBUG_CULLSETTINGS
    osg::notify(osg::NOTICE)<<"CullVisitor, after inheritCullSettings(saved_cull_settings,"<<camera.getInheritanceMask()<<") : ";
    write(osg::notify(osg::NOTICE));
#endif

#else
    // activate all active cull settings from this Camera
    inheritCullSettings(camera);
#endif

    // set the cull mask.
    unsigned int savedTraversalMask = getTraversalMask();
    bool mustSetCullMask = (camera.getInheritanceMask()
            & osg::CullSettings::CULL_MASK) == 0;
    if(mustSetCullMask)
        setTraversalMask(camera.getCullMask());

    RefMatrix& originalModelView = *getModelViewMatrix();

    osg::RefMatrix* projection = 0;
    osg::RefMatrix* modelview = 0;

    if(camera.getReferenceFrame() == osg::Transform::RELATIVE_RF)
    {
        if(camera.getTransformOrder() == osg::Camera::POST_MULTIPLY)
        {
            projection = createOrReuseMatrix(
                    *getProjectionMatrix() * camera.getProjectionMatrix());
            modelview = createOrReuseMatrix(
                    *getModelViewMatrix() * camera.getViewMatrix());
        }
        else // pre multiply 
        {
            projection = createOrReuseMatrix(
                    camera.getProjectionMatrix() * (*getProjectionMatrix()));
            modelview = createOrReuseMatrix(
                    camera.getViewMatrix() * (*getModelViewMatrix()));
        }
    }
    else
    {
        // an absolute reference frame
        projection = createOrReuseMatrix(camera.getProjectionMatrix());
        modelview = createOrReuseMatrix(camera.getViewMatrix());
    }

    if(camera.getViewport())
        pushViewport(camera.getViewport());

    // record previous near and far values.
    float previous_znear = _computed_znear;
    float previous_zfar = _computed_zfar;

    // take a copy of the current near plane candidates
    DistanceMatrixDrawableMap previousNearPlaneCandidateMap;
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;

    pushProjectionMatrix(projection);
    pushModelViewMatrix(modelview,camera.getReferenceFrame());

    if(camera.getRenderOrder() == osg::Camera::NESTED_RENDER)
    {
        handle_cull_callbacks_and_traverse(camera);
    }
    else
    {
        // set up lighting.
        // currently ignore lights in the scene graph itself..
        // will do later.
        osgUtil::RenderStage* previous_stage =
                getCurrentRenderBin()->getStage();

//        unsigned int contextID = getState() ? getState()->getContextID() : 0;

        // use render to texture stage.
        // create the render to texture stage.
        osg::ref_ptr<osgUtil::RenderStageCache> rsCache =
                dynamic_cast<osgUtil::RenderStageCache*>(camera.getRenderingCache());
        if(!rsCache)
        {
            rsCache = new osgUtil::RenderStageCache;
            camera.setRenderingCache(rsCache.get());
        }

        osg::ref_ptr < osgUtil::RenderStage > rtts = rsCache->getRenderStage(
                this);
        if(!rtts)
        {
            OpenThreads::ScopedLock < OpenThreads::Mutex
                    > lock(*(camera.getDataChangeMutex()));

            rtts = new osgUtil::RenderStage;
            rsCache->setRenderStage(this,rtts.get());

            rtts->setCamera(&camera);

            if(camera.getInheritanceMask() & DRAW_BUFFER)
            {
                // inherit draw buffer from above.
                rtts->setDrawBuffer(previous_stage->getDrawBuffer(),
                        previous_stage->getDrawBufferApplyMask());
            }
            else
            {
                rtts->setDrawBuffer(camera.getDrawBuffer());
            }

            if(camera.getInheritanceMask() & READ_BUFFER)
            {
                // inherit read buffer from above.
                rtts->setReadBuffer(previous_stage->getReadBuffer(),
                        previous_stage->getReadBufferApplyMask());
            }
            else
            {
                rtts->setReadBuffer(camera.getReadBuffer());
            }
        }
        else
        {
            // reusing render to texture stage, so need to reset it to empty it from previous frames contents.
            rtts->reset();
        }

        // set up clera masks/values        
        rtts->setClearDepth(camera.getClearDepth());
        rtts->setClearAccum(camera.getClearAccum());
        rtts->setClearStencil(camera.getClearStencil());
        rtts->setClearMask(camera.getClearMask());

        // set up the background color and clear mask.
        if(camera.getInheritanceMask() & CLEAR_COLOR)
        {
            rtts->setClearColor(previous_stage->getClearColor());
        }
        else
        {
            rtts->setClearColor(camera.getClearColor());
        }
        if(camera.getInheritanceMask() & CLEAR_MASK)
        {
            rtts->setClearMask(previous_stage->getClearMask());
        }
        else
        {
            rtts->setClearMask(camera.getClearMask());
        }

        // set the color mask.
        osg::ColorMask* colorMask =
                camera.getColorMask() != 0 ?
                        camera.getColorMask() : previous_stage->getColorMask();
        rtts->setColorMask(colorMask);

        // set up the viewport.
        osg::Viewport* viewport =
                camera.getViewport() != 0 ?
                        camera.getViewport() : previous_stage->getViewport();
        rtts->setViewport(viewport);

        // set up to charge the same PositionalStateContainer is the parent previous stage.
        osg::Matrix inheritedMVtolocalMV;
        inheritedMVtolocalMV.invert(originalModelView);
        inheritedMVtolocalMV.postMult(*getModelViewMatrix());
        rtts->setInheritedPositionalStateContainerMatrix(inheritedMVtolocalMV);
        rtts->setInheritedPositionalStateContainer(
                previous_stage->getPositionalStateContainer());

        // record the render bin, to be restored after creation
        // of the render to text
        osgUtil::RenderBin* previousRenderBin = getCurrentRenderBin();

        // set the current renderbin to be the newly created stage.
        setCurrentRenderBin(rtts.get());

        // traverse the subgraph
        {
            handle_cull_callbacks_and_traverse(camera);
        }

        // restore the previous renderbin.
        setCurrentRenderBin(previousRenderBin);

        if(rtts->getStateGraphList().size() == 0
                && rtts->getRenderBinList().size() == 0)
        {
            // getting to this point means that all the subgraph has been
            // culled by small feature culling or is beyond LOD ranges.
        }

        // and the render to texture stage to the current stages
        // dependancy list.
        switch(camera.getRenderOrder())
        {
            case osg::Camera::PRE_RENDER:
                getCurrentRenderBin()->getStage()->addPreRenderStage(rtts.get(),
                        camera.getRenderOrderNum());
                break;
            default:
                getCurrentRenderBin()->getStage()->addPostRenderStage(
                        rtts.get(),camera.getRenderOrderNum());
                break;
        }

    }

    // restore the previous model view matrix.
    popModelViewMatrix();

    // restore the previous model view matrix.
    popProjectionMatrix();

    // restore the original near and far values
    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;

    // swap back the near plane candidates
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    if(camera.getViewport())
        popViewport();

    // restore the previous traversal mask settings
    if(mustSetCullMask)
        setTraversalMask(savedTraversalMask);

    // restore the previous cull settings
    setCullSettings(saved_cull_settings);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

}

void MultiViewerCullVisitor::apply(osg::OccluderNode& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;

    // need to check if occlusion node is in the occluder
    // list, if so disable the appropriate ShadowOccluderVolume
    disableAndPushOccludersCurrentMask(_nodePath);

    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        popOccludersCurrentMask(_nodePath);
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();

    // pop the current mask for the disabled occluder
    popOccludersCurrentMask(_nodePath);

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::apply(osg::OcclusionQueryNode& node)
{
    bool status = _cullingStatus;
    bool firstStatus = _firstCullStatus;

    if(isCulled(node))
    {
        _firstCullStatus = firstStatus;
        _cullingStatus = status;
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if(node_state)
        pushStateSet(node_state);

    osg::Camera* camera = getCurrentCamera();

    // If previous query indicates visible, then traverse as usual.
#if (OPENSCENEGRAPH_MAJOR_VERSION == 2) && (OPENSCENEGRAPH_MINOR_VERSION == 9) && (OPENSCENEGRAPH_PATCH_VERSION <= 7)
    if (node.getPassed( camera, getDistanceToEyePoint( node.getBound()._center, false ) ))
#else
    if(node.getPassed(camera,*this))
#endif
        handle_cull_callbacks_and_traverse(node);

    // Traverse the query subtree if OcclusionQueryNode needs to issue another query.
    node.traverseQuery(camera,*this);

    // Traverse the debug bounding geometry, if enabled.
    node.traverseDebug(*this);

    // pop the node's state off the render graph stack.    
    if(node_state)
        popStateSet();

    // pop the culling mode.
    popCurrentMask();

    _firstCullStatus = firstStatus;
    _cullingStatus = status;
}

void MultiViewerCullVisitor::pushModelViewMatrix(RefMatrix* matrix,
        Transform::ReferenceFrame referenceFrame)
{
    //std::cerr << "Push Matrix" << std::endl;
    osg::RefMatrix* originalModelView =
            _modelviewStack.empty() ? 0 : _modelviewStack.back().get();

    _modelviewStack.push_back(matrix);

    pushCullingSet();

    osg::Matrix inv;
    inv.invert(*matrix);

    //osg::Vec3 org(0,0,0);
    //org = org * (*matrix);

    //std::cerr << "World space of origin x: " << org.x() << " y: " << org.y() << " z: " << org.z() << std::endl;

    _invMVStack.push(*matrix);
    _currentNearFrustum.setAndTransformProvidingInverse(_nearFrustum,*matrix);
    _currentFarFrustum.setAndTransformProvidingInverse(_farFrustum,*matrix);

    switch(referenceFrame)
    {
        case (Transform::RELATIVE_RF):
            _eyePointStack.push_back(inv.getTrans());
            _referenceViewPoints.push_back(getReferenceViewPoint());
            _viewPointStack.push_back(getReferenceViewPoint() * inv);
            break;
        case (Transform::ABSOLUTE_RF):
            _eyePointStack.push_back(inv.getTrans());
            _referenceViewPoints.push_back(osg::Vec3(0.0,0.0,0.0));
            _viewPointStack.push_back(_eyePointStack.back());
            break;
        case (Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT):
        {
            _eyePointStack.push_back(inv.getTrans());

            osg::Vec3 referenceViewPoint = getReferenceViewPoint();
            if(originalModelView)
            {
                osg::Matrix viewPointTransformMatrix;
                viewPointTransformMatrix.invert(*originalModelView);
                viewPointTransformMatrix.postMult(*matrix);
                referenceViewPoint = referenceViewPoint
                        * viewPointTransformMatrix;
            }

            _referenceViewPoints.push_back(referenceViewPoint);
            _viewPointStack.push_back(getReferenceViewPoint() * inv);
            break;
        }
    }

    osg::Vec3 lookVector = getLookVectorLocal();

    _bbCornerFar = (lookVector.x() >= 0 ? 1 : 0) | (lookVector.y() >= 0 ? 2 : 0)
            | (lookVector.z() >= 0 ? 4 : 0);

    _bbCornerNear = (~_bbCornerFar) & 7;

}

void MultiViewerCullVisitor::popModelViewMatrix()
{
    //std::cerr << "Pop Matrix" << std::endl;
    _modelviewStack.pop_back();

    _eyePointStack.pop_back();
    _referenceViewPoints.pop_back();
    _viewPointStack.pop_back();

    popCullingSet();

    _invMVStack.pop();

    if(_invMVStack.size())
    {
        _currentNearFrustum.setAndTransformProvidingInverse(_nearFrustum,
                _invMVStack.top());
        _currentFarFrustum.setAndTransformProvidingInverse(_farFrustum,
                _invMVStack.top());
    }

    osg::Vec3 lookVector(0.0f,0.0f,-1.0f);
    if(!_modelviewStack.empty())
    {
        lookVector = getLookVectorLocal();
    }
    _bbCornerFar = (lookVector.x() >= 0 ? 1 : 0) | (lookVector.y() >= 0 ? 2 : 0)
            | (lookVector.z() >= 0 ? 4 : 0);

    _bbCornerNear = (~_bbCornerFar) & 7;
}

void MultiViewerCullVisitor::setFrustums(osg::Polytope & near,
        osg::Polytope & far)
{
    _nearFrustum = near;
    _farFrustum = far;
    _currentNearFrustum = near;
    _currentFarFrustum = far;

    if(_invMVStack.size())
    {
        _currentNearFrustum.setAndTransformProvidingInverse(_nearFrustum,
                _invMVStack.top());
        _currentFarFrustum.setAndTransformProvidingInverse(_farFrustum,
                _invMVStack.top());
    }
}
