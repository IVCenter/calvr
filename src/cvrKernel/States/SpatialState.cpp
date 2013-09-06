/*
 * SpatialState.cpp
 *
 */

#include <vector>
#include <cvrKernel/States/SpatialState.h>

using namespace cvr;

/*static*/ std::string const SpatialState::TYPE = "spatial";

SpatialState::SpatialState() : CvrState(TYPE)
{
    osg::Vec3 default_vec3;
    osg::Quat default_vec4;

    Position(default_vec3);
    Scale(default_vec3);
    Rotation(default_vec4);

    Navigation(false);

    Metadata("");
    Parent("");
}

/*static*/ CvrState*
SpatialState::Adapter( State const& state )
{
    return new SpatialState( state );
}

osg::Vec3 const
SpatialState::Position(void)
{
    std::vector<double> pos;
    GetVariable("position", pos);

    return osg::Vec3(pos[0], pos[1], pos[2]);
}

void
SpatialState::Position(osg::Vec3 const& position)
{
    std::vector<double> vec3;
    vec3.push_back(position.x());
    vec3.push_back(position.y());
    vec3.push_back(position.z());

    SetVariable("position", vec3);
}

osg::Quat const
SpatialState::Rotation(void)
{
    std::vector<double> vec4;
    GetVariable("rotation", vec4);

    return osg::Quat( osg::Vec4d(vec4[0], vec4[1], vec4[2], vec4[3]) );
}

void
SpatialState::Rotation(osg::Quat const& rotation)
{
    std::vector<double> vec4;
    vec4.push_back(rotation.x());
    vec4.push_back(rotation.y());
    vec4.push_back(rotation.z());
    vec4.push_back(rotation.w());

    SetVariable("rotation", vec4);
}

osg::Vec3 const
SpatialState::Scale(void)
{
    std::vector<double> scale;
    GetVariable("scale", scale);

    return osg::Vec3(scale[0], scale[1], scale[2]);
}

void
SpatialState::Scale(osg::Vec3 const& scale)
{
    std::vector<double> vec3;
    vec3.push_back(scale.x());
    vec3.push_back(scale.y());
    vec3.push_back(scale.z());

    SetVariable("scale", vec3);
}

bool const
SpatialState::Navigation(void)
{
    bool navigation;
    GetVariable("navigation", navigation);

    return navigation;
}

void
SpatialState::Navigation(bool const navigation)
{
    SetVariable("navigation", navigation);
}

std::string const
SpatialState::Metadata(void)
{
    std::string meta_uuid;
    GetVariable("metadata", meta_uuid);

    return meta_uuid;
}

void
SpatialState::Metadata(std::string const& metadata)
{
    SetVariable("metadata", metadata);
}

std::string const
SpatialState::Parent(void)
{
    std::string parent_uuid;
    GetVariable("parent", parent_uuid);

    return parent_uuid;
}

void
SpatialState::Parent(std::string const& spatial)
{
    SetVariable("parent", spatial);
}

// PROTECTED FUNCTIONS

SpatialState::SpatialState(State const& state) : CvrState(state)
{}

/*virtual*/
SpatialState::~SpatialState()
{}

