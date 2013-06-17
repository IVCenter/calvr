#include <cvrKernel/ComController.h>
#include <cvrKernel/PluginManager.h>
#include <cvrInput/Gesture.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
using namespace cvr;

#include <rk.h>
#include <rkgs.h>

namespace {

struct State
{
    RKGestureContext* gesture_context;
    RKClient*         client;
};

string slurp(const string& filename)
{
    ifstream src(filename.c_str());
    stringstream buffer;
    buffer << src.rdbuf();
    
    return buffer.str();
}

bool translate_skeletons(RKClient* client, RKSkeletonFrame& sf)
{
    bool got_frame = false;
    
    while(rk_client_recv_skeleton(client, &sf))
    {
        // read all frames that are waiting
        got_frame = true;
    }
    
    if(!got_frame)
        return false;

    return true;
}


} // anonymous namespace

namespace cvr {

void GestureScript::reset()
{
    RKGestureContext* context = 
        ((State*)detector->internal_state)->gesture_context;
        
    rkgs_kill(context, name.c_str());
    rkgs_load(context, name.c_str(), code.c_str());
}

void GestureScript::suspend()
{
    RKGestureContext* context = 
        ((State*)detector->internal_state)->gesture_context;
        
    rkgs_suspend(context, name.c_str());
}

void GestureScript::resume()
{
    RKGestureContext* context = 
        ((State*)detector->internal_state)->gesture_context;
        
    rkgs_resume(context, name.c_str());
}

void GestureScript::cancel()
{
    RKGestureContext* context = 
        ((State*)detector->internal_state)->gesture_context;
        
    rkgs_kill(context, name.c_str());
}

GestureScript::GestureScript() {}


// -------------------------------------------------------------------------

GestureDetector::GestureDetector(std::string host, unsigned short port)
{
    State* state = new State();
    state->gesture_context = rkgs_init();
    state->client          = rk_client_init(host.c_str(),0, 0, port);
    internal_state = (void*)state;
}

GestureDetector::~GestureDetector()
{
    State* state = (State*)internal_state;
    rkgs_shutdown(state->gesture_context);
    rk_client_shutdown(state->client);
    delete state;
    
    internal_state = NULL;
}

GestureScript GestureDetector::load_string(string name, string code)
{
    RKGestureContext* context = ((State*)internal_state)->gesture_context;
    rkgs_load(context, name.c_str(), code.c_str());
    
    GestureScript script;
    return script;
}

GestureScript GestureDetector::load_file(string filename)
{
    string code = slurp(filename);
    
    RKGestureContext* context = ((State*)internal_state)->gesture_context;
    rkgs_load(context, filename.c_str(), code.c_str());
    
    GestureScript script;
    return script;
}

void GestureDetector::look_in_plugin(string name)
{
    RKGestureContext* context = ((State*)internal_state)->gesture_context;
    
    string path = PluginManager::instance()->getPathOfPlugin(name);
    if(path == "")
    {
        fprintf(stderr, "Can't find path to %s\n", name.c_str());
        return;
    }
    
    rkgs_search_for_symbols(context, name.c_str(), path.c_str());
}

void GestureDetector::step()
{
    State* state = (State*)internal_state;
    RKSkeletonFrame sf;
    
    ComController* com = ComController::instance();
    
    bool dispatch;
    
    if(com->isMaster())
    {
        dispatch = translate_skeletons(state->client, sf);
        com->sendSlaves(&dispatch, sizeof(dispatch));
        
        if(dispatch)
        {
            com->sendSlaves(&sf, sizeof(sf));
        }
    }
    else
    {
        com->readMaster(&dispatch, sizeof(dispatch));
        
        if(dispatch)
        {
            com->readMaster(&sf, sizeof(sf));
        }
    }
    
    if(dispatch)
    {
        rkgs_input(state->gesture_context, &sf);
        rkgs_step(state->gesture_context);
    }
}


} //namespace cvr
