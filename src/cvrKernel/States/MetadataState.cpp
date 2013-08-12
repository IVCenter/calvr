/*
 * MetadataState.cpp
 *
 */

#include <vector>
#include <cvrKernel/States/MetadataState.h>
#include <iostream>

std::string const MetadataState::TYPE = "metadata";

MetadataState::MetadataState() : CvrState(TYPE)
{
    osg::Vec3 default_vec;

    Volume(default_vec);
    Path("");

    Register();
}

/*static*/ CvrState*
MetadataState::Adapter(State const& state)
{
    return new MetadataState(state);
}

/*static*/ void
MetadataState::Register(void)
{
    static bool FIRST_ONE = true;
    if (FIRST_ONE)
    {
        FIRST_ONE = false;
        CvrState::Register(TYPE, &Adapter);
    }
}

std::string const
MetadataState::Path(void)
{
    std::string path;
    GetVariable("path", path);

    return path;
}

void
MetadataState::Path(std::string const& path)
{
    SetVariable("path", path);
}

osg::Vec3 const
MetadataState::Volume(void)
{
    std::vector<double> volume;
    GetVariable("volume", volume);

    return osg::Vec3(volume[0], volume[1], volume[2]);
}

void
MetadataState::Volume(osg::Vec3 const& volume)
{
    std::vector<double> vec3;
    vec3.push_back(volume.x());
    vec3.push_back(volume.y());
    vec3.push_back(volume.z());
    SetVariable("volume", vec3);
}

// PROTECTED FUNCTIONS

MetadataState::MetadataState(State const& state) : CvrState(state)
{
}

/*virtual*/
MetadataState::~MetadataState()
{}

