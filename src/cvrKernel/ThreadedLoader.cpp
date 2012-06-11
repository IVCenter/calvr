#include <cvrKernel/ThreadedLoader.h>
#include <cvrKernel/ComController.h>
#include <cvrKernel/CVRViewer.h>

#include <iostream>

using namespace cvr;

ThreadedLoader * ThreadedLoader::_myPtr = NULL;

ThreadedLoader::ThreadedLoader()
{
    _jobID = 0;
}

ThreadedLoader::~ThreadedLoader()
{
    std::map<int,char>::iterator it;
    while((it = _jobStatus.begin()) != _jobStatus.end())
    {
        if(_threads[it->first]->isRunning())
        {
            _threads[it->first]->quit();
            _threads[it->first]->join();
        }
        remove(it->first);
    }

    for(std::map<int,LoaderThread*>::iterator it2 = _threads.begin();
            it2 != _threads.end(); it2++)
    {
        it2->second->join();
        delete it2->second;
    }
    _threads.clear();
}

ThreadedLoader * ThreadedLoader::instance()
{
    if(!_myPtr)
    {
        _myPtr = new ThreadedLoader();
    }
    return _myPtr;
}

int ThreadedLoader::readNodeFile(std::string filename,
        osgDB::ReaderWriter::Options * options)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = READ_NODE;
    tj->strs.push_back(filename);
    tj->options.push_back(options);

    _jobs[_jobID] = tj;
    _threads[_jobID] = new LoaderThread(tj);
    _jobStatus[_jobID] = 0;
    _threads[_jobID]->start();

    _jobID++;
    return _jobID - 1;
}

int ThreadedLoader::readNodeFiles(std::vector<std::string> & filenames,
        std::vector<osgDB::ReaderWriter::Options*> & options)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = READ_NODE_LIST;
    tj->strs = filenames;
    tj->options = options;

    /*for(int i = 0; i < options.size(); i++)
     {
     tj->options.push_back(options[i]);
     }*/

    _jobs[_jobID] = tj;
    _threads[_jobID] = new LoaderThread(tj);
    _jobStatus[_jobID] = 0;
    _threads[_jobID]->start();

    _jobID++;
    return _jobID - 1;
}

int ThreadedLoader::readImageFile(std::string & filename,
        osgDB::ReaderWriter::Options * options)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = READ_IMAGE;
    tj->strs.push_back(filename);
    tj->options.push_back(options);

    _jobs[_jobID] = tj;
    _threads[_jobID] = new LoaderThread(tj);
    _jobStatus[_jobID] = 0;
    _threads[_jobID]->start();

    _jobID++;
    return _jobID - 1;
}

int ThreadedLoader::readImageFiles(std::vector<std::string> & filenames,
        std::vector<osgDB::ReaderWriter::Options*> & options)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = READ_IMAGE_LIST;
    tj->strs = filenames;

    for(int i = 0; i < options.size(); i++)
    {
        tj->options.push_back(options[i]);
    }

    _jobs[_jobID] = tj;
    _threads[_jobID] = new LoaderThread(tj);
    _jobStatus[_jobID] = 0;
    _threads[_jobID]->start();

    _jobID++;
    return _jobID - 1;
}

int ThreadedLoader::systemCommand(std::string command)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = SYSTEM;
    tj->strs.push_back(command);

    _jobs[_jobID] = tj;
    _threads[_jobID] = new LoaderThread(tj);
    _jobStatus[_jobID] = 0;
    _threads[_jobID]->start();

    _jobID++;
    return _jobID - 1;
}

int ThreadedLoader::systemCommandMasterOnly(std::string command)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = SYSTEM_MASTER;
    tj->strs.push_back(command);

    _jobs[_jobID] = tj;
    _threads[_jobID] = new LoaderThread(tj);
    _jobStatus[_jobID] = 0;
    _threads[_jobID]->start();

    _jobID++;
    return _jobID - 1;
}

bool ThreadedLoader::isError(int job)
{
    if(_jobStatus.find(job) == _jobStatus.end())
    {
        return true;
    }

    return _jobStatus[job] == -1;
}

bool ThreadedLoader::isDone(int job)
{
    if(_jobStatus.find(job) == _jobStatus.end())
    {
        return false;
    }

    return _jobStatus[job] == 127;
}

float ThreadedLoader::getProgress(int job)
{
    if(_jobStatus.find(job) == _jobStatus.end())
    {
        return -1;
    }

    if(_jobStatus[job] == -1)
    {
        return -1;
    }

    return ((float)_jobStatus[job]) / 127.0f;
}

void ThreadedLoader::remove(int job)
{
    if(_jobStatus.find(job) == _jobStatus.end())
    {
        return;
    }

    _jobStatus.erase(job);
    if(!_threads[job]->isRunning())
    {
        delete _threads[job];
        _threads.erase(job);
    }
    else
    {
        _threads[job]->quit();
    }

    if(_jobs.find(job) != _jobs.end())
    {
        //delete _jobs[job];
        _jobs.erase(job);
    }
}

void ThreadedLoader::getNodeFile(int job, osg::ref_ptr<osg::Node> & node)
{
    if(_threads.find(job) == _threads.end())
    {
        std::cerr << "ThreadedLoader Error: getNodeFile, job: " << job
                << " does not exist." << std::endl;
        node = NULL;
        return;
    }

    if(!isDone(job))
    {
        std::cerr << "ThreadedLoader Error: job: " << job << " is not done yet."
                << std::endl;
        node = NULL;
        return;
    }

    if(_threads[job]->getResult()->type != READ_NODE
            && _threads[job]->getResult()->type != READ_NODE_LIST)
    {
        std::cerr << "ThreadedLoader Error: getNodeFile, job: " << job
                << " is not of reading a node file." << std::endl;
        node = NULL;
        return;
    }

    if(_threads[job]->getResult()->ptrs.size())
    {
        node = (osg::Node*)_threads[job]->getResult()->ptrs[0];
    }
    else
    {
        node = NULL;
    }
}

void ThreadedLoader::getNodeFiles(int job,
        std::vector<osg::ref_ptr<osg::Node> > & nodeList)
{
    if(_threads.find(job) == _threads.end())
    {
        std::cerr << "ThreadedLoader Error: getNodeFiles, job: " << job
                << " does not exist." << std::endl;
        return;
    }

    if(!isDone(job))
    {
        std::cerr << "ThreadedLoader Error: job: " << job << " is not done yet."
                << std::endl;
        return;
    }

    if(_threads[job]->getResult()->type != READ_NODE
            && _threads[job]->getResult()->type != READ_NODE_LIST)
    {
        std::cerr << "ThreadedLoader Error: getNodeFiles, job: " << job
                << " is not of reading a node file." << std::endl;
        return;
    }

    for(int i = 0; i < _threads[job]->getResult()->ptrs.size(); i++)
    {
        nodeList.push_back((osg::Node*)_threads[job]->getResult()->ptrs[i]);
    }
}

void ThreadedLoader::getImageFile(int job, osg::ref_ptr<osg::Image> & image)
{
    if(_threads.find(job) == _threads.end())
    {
        std::cerr << "ThreadedLoader Error: getImageFile, job: " << job
                << " does not exist." << std::endl;
        image = NULL;
        return;
    }

    if(!isDone(job))
    {
        std::cerr << "ThreadedLoader Error: job: " << job << " is not done yet."
                << std::endl;
        image = NULL;
        return;
    }

    if(_threads[job]->getResult()->type != READ_IMAGE
            && _threads[job]->getResult()->type != READ_IMAGE_LIST)
    {
        std::cerr << "ThreadedLoader Error: getImageFile, job: " << job
                << " is not of reading an image file." << std::endl;
        image = NULL;
        return;
    }

    if(_threads[job]->getResult()->ptrs.size())
    {
        image = (osg::Image*)_threads[job]->getResult()->ptrs[0];
    }
    else
    {
        image = NULL;
    }
}

void ThreadedLoader::getImageFiles(int job,
        std::vector<osg::ref_ptr<osg::Image> > & imageList)
{
    if(_threads.find(job) == _threads.end())
    {
        std::cerr << "ThreadedLoader Error: getImageFiles, job: " << job
                << " does not exist." << std::endl;
        return;
    }

    if(!isDone(job))
    {
        std::cerr << "ThreadedLoader Error: job: " << job << " is not done yet."
                << std::endl;
        return;
    }

    if(_threads[job]->getResult()->type != READ_IMAGE
            && _threads[job]->getResult()->type != READ_IMAGE_LIST)
    {
        std::cerr << "ThreadedLoader Error: getImageFiles, job: " << job
                << " is not of reading an image file." << std::endl;
        return;
    }

    for(int i = 0; i < _threads[job]->getResult()->ptrs.size(); i++)
    {
        imageList.push_back((osg::Image*)_threads[job]->getResult()->ptrs[i]);
    }
}

void ThreadedLoader::update()
{
    double startTime, endTime;

    osg::Stats * stats;
    stats = CVRViewer::instance()->getViewerStats();
    if(stats && !stats->collectStats("CalVRStatsAdvanced"))
    {
	stats = NULL;
    }

    if(stats)
    {
	startTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
    }

    if(!_jobs.size())
    {
	if(stats)
	{
	    endTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
	    stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "TLoader begin time", startTime);
	    stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "TLoader end time", endTime);
	    stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "TLoader time taken", endTime-startTime);
	}
	return;
    }

    char * status = new char[_jobs.size()];

    int index = 0;
    for(std::map<int,ThreadedJob*>::iterator it = _jobs.begin();
            it != _jobs.end(); it++)
    {
        status[index] = _threads[it->first]->getStatus();
        index++;
    }

    if(ComController::instance()->isMaster())
    {
        char * slaveStatus = new char[ComController::instance()->getNumSlaves()
                * _jobs.size() * sizeof(char)];

        ComController::instance()->readSlaves(slaveStatus,
                _jobs.size() * sizeof(char));

        int numSlaves = ComController::instance()->getNumSlaves();
        int index = 0;
        for(std::map<int,ThreadedJob*>::iterator it = _jobs.begin();
                it != _jobs.end(); it++)
        {
            for(int i = 0; i < numSlaves; i++)
            {
                status[index] = std::min(status[index],
                        slaveStatus[(i * _jobs.size()) + index]);
            }
            index++;
        }

        ComController::instance()->sendSlaves(status,
                _jobs.size() * sizeof(char));

        delete[] slaveStatus;
    }
    else
    {
        ComController::instance()->sendMaster(status,
                _jobs.size() * sizeof(char));
        ComController::instance()->readMaster(status,
                _jobs.size() * sizeof(char));
    }

    std::vector<int> eraseList;

    index = 0;
    for(std::map<int,ThreadedJob*>::iterator it = _jobs.begin();
            it != _jobs.end(); it++)
    {
        if(status[index] == -1)
        {
            if(_threads[it->first]->isRunning())
            {
                _threads[it->first]->cancel();
            }
            eraseList.push_back(it->first);
        }
        else if(status[index] == 127)
        {
            eraseList.push_back(it->first);
        }

        _jobStatus[it->first] = status[index];

        index++;
    }

    for(int i = 0; i < eraseList.size(); i++)
    {
        //delete _jobs[eraseList[i]];
        _jobs.erase(eraseList[i]);
    }

    delete[] status;

    // look for threads that still need to finish
    for(std::map<int,LoaderThread*>::iterator it = _threads.begin();
            it != _threads.end(); it++)
    {
        if(_jobStatus.find(it->first) == _jobStatus.end())
        {
            if(!it->second->isRunning())
            {
                //std::cerr << "Removing stray thread." << std::endl;
                delete it->second;
                _threads.erase(it);
                break;
            }
        }
    }

    if(stats)
    {
	endTime = osg::Timer::instance()->delta_s(CVRViewer::instance()->getStartTick(), osg::Timer::instance()->tick());
	stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "TLoader begin time", startTime);
	stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "TLoader end time", endTime);
	stats->setAttribute(CVRViewer::instance()->getViewerFrameStamp()->getFrameNumber(), "TLoader time taken", endTime-startTime);
    }
}

ThreadedLoader::LoaderThread::LoaderThread(ThreadedJob * job)
{
    _job = job;
    _result.type = job->type;
    _result.ret = 0;
    _status = 0;
    _quit = false;
}

ThreadedLoader::LoaderThread::~LoaderThread()
{
    for(int i = 0; i < _result.ptrs.size(); i++)
    {
        if(_result.type == READ_NODE || _result.type == READ_NODE_LIST)
        {
            ((osg::Node*)_result.ptrs[i])->unref();
        }
        else if(_result.type == READ_IMAGE || _result.type == READ_IMAGE_LIST)
        {
            ((osg::Image*)_result.ptrs[i])->unref();
        }
    }
    delete _job;
}

void ThreadedLoader::LoaderThread::run()
{
    switch(_job->type)
    {
        case READ_NODE:
        case READ_NODE_LIST:
        {
            osg::Node * node;
            for(int i = 0; i < _job->strs.size(); i++)
            {
                if(_job->options.size() > i && _job->options[i])
                {
                    node = osgDB::readNodeFile(_job->strs[i],_job->options[i]);
                    if(node)
                    {
                        node->ref();
                    }
                    _result.ptrs.push_back((void*)node);
                }
                else
                {
                    node = osgDB::readNodeFile(_job->strs[i]);
                    if(node)
                    {
                        node->ref();
                    }
                    _result.ptrs.push_back((void*)node);
                }

                _quitLock.lock();
                if(_quit)
                {
                    _quitLock.unlock();
                    break;
                }
                _quitLock.unlock();

                _statusLock.lock();
                _status = ((char)(((float)i + 1.0) / ((float)_job->strs.size())
                        * 126.0));
                _statusLock.unlock();

            }
            break;
        }
        case READ_IMAGE:
        case READ_IMAGE_LIST:
        {
            for(int i = 0; i < _job->strs.size(); i++)
            {
                osg::Image * image;
                if(_job->options.size() > i && _job->options[i])
                {
                    image = osgDB::readImageFile(_job->strs[i],
                            _job->options[i]);\
                    if(image)
                    {
                        image->ref();
                    }
                    _result.ptrs.push_back((void*)image);
                }
                else
                {
                    image = osgDB::readImageFile(_job->strs[i]);
                    if(image)
                    {
                        image->ref();
                    }
                    _result.ptrs.push_back((void*)image);
                }

                _quitLock.lock();
                if(_quit)
                {
                    _quitLock.unlock();
                    break;
                }
                _quitLock.unlock();

                _statusLock.lock();
                _status = ((char)(((float)i + 1.0) / ((float)_job->strs.size())
                        * 126.0));
                _statusLock.unlock();
            }
            break;
        }
        case SYSTEM:
        {
            if(_job->strs.size())
            {
                system(_job->strs[0].c_str());
            }
            break;
        }
        case SYSTEM_MASTER:
        {
            if(ComController::instance()->isMaster())
            {
                if(_job->strs.size())
                {
                    system(_job->strs[0].c_str());
                }
            }
            break;
        }
        default:
            break;
    }

    _statusLock.lock();
    _status = 127;
    _statusLock.unlock();
}

void ThreadedLoader::LoaderThread::quit()
{
    _quitLock.lock();
    _quit = true;
    _quitLock.unlock();
}

char ThreadedLoader::LoaderThread::getStatus()
{
    char ret;

    _statusLock.lock();

    ret = _status;

    _statusLock.unlock();

    return ret;
}
