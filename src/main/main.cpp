#include <kernel/CalVR.h>
#include <iostream>
#include <cstdlib>

#include <osg/ArgumentParser>

int main(int argc, char **argv)
{
    char * cvrDir = getenv("CALVR_HOME");
    if(!cvrDir)
    {
        std::cerr
                << "Error: CALVR_HOME environment variable not set.  Quitting."
                << std::endl;
        return 0;
    }

    cvr::CalVR * calvr = new cvr::CalVR();
    osg::ArgumentParser args(&argc, argv);
    if(!calvr->init(args,cvrDir))
    {
	return 0;
    }

    calvr->run();
    delete calvr;

    std::cerr << "Main done." << std::endl;
    return 1;
}
