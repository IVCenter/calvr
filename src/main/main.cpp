#include <cvrKernel/CalVR.h>
#include <iostream>
#include <cstdlib>

#include <osg/ArgumentParser>

int main(int argc, char **argv)
{
    osg::ArgumentParser args(&argc,argv);

	/*
    args.getApplicationUsage()->setApplicationName(args.getApplicationName());
    args.getApplicationUsage()->setDescription(
            args.getApplicationName()
                    + " is and OpenSceneGraph based virtual reality framework.");
    args.getApplicationUsage()->setCommandLineUsage(
            args.getApplicationName() + " [options] [files to open]");
    args.getApplicationUsage()->addCommandLineOption("--host-name <name>",
            "String used to identify this host in config files, etc. default: gethostname()");
    args.getApplicationUsage()->addCommandLineOption("-h or --help",
            "Display command line parameters");
			*/

    if(args.read("-h") || args.read("--help"))
    {
        args.getApplicationUsage()->write(std::cout);
        return 0;
    }

    char * cvrDir = getenv("CALVR_HOME");
    if(!cvrDir)
    {
        std::cerr
                << "Error: CALVR_HOME environment variable not set.  Quitting."
                << std::endl;
        return 0;
    }

    cvr::CalVR * calvr = new cvr::CalVR();
    if(!calvr->init(args,cvrDir))
    {
        delete calvr;
        return 0;
    }

    calvr->run();
    delete calvr;

    std::cerr << "Main done." << std::endl;
    return 1;
}
