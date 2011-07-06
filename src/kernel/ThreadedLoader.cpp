#include <kernel/ThreadedLoader.h>

#include <iostream>

using namespace cvr;

ThreadedLoader * ThreadedLoader::_myPtr = NULL;

ThreadedLoader::ThreadedLoader()
{
    _jobID = 0;
}

ThreadedLoader::~ThreadedLoader()
{
}

ThreadedLoader * ThreadedLoader::instance()
{
    if(!_myPtr)
    {
	_myPtr = new ThreadedLoader();
    }
    return _myPtr;
}

int ThreadedLoader::readNodeFile(std::string & filename, osgDB::ReaderWriter::Options * options)
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

int ThreadedLoader::readNodeFiles(std::vector<std::string> & filenames, std::vector<osgDB::ReaderWriter::Options*> & options)
{
    ThreadedJob * tj = new ThreadedJob;
    tj->type = READ_NODE_LIST;
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

int ThreadedLoader::readImageFile(std::string & filename, osgDB::ReaderWriter::Options * options)
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

int ThreadedLoader::readImageFiles(std::vector<std::string> & filenames, std::vector<osgDB::ReaderWriter::Options*> & options)
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
    delete _threads[job];
    _threads.erase(job);
}

void ThreadedLoader::getNodeFile(osg::Node * & node)
{
} 

void ThreadedLoader::getNodeFiles(std::vector<osg::Node*> & nodeList)
{
}

void ThreadedLoader::getImageFile(osg::Image * & image)
{
}

void ThreadedLoader::getImageFiles(std::vector<osg::Image*> & imageList)
{
}

void ThreadedLoader::update()
{
}

ThreadedLoader::LoaderThread::LoaderThread(ThreadedJob * job)
{
}

ThreadedLoader::LoaderThread::~LoaderThread()
{
}

void ThreadedLoader::LoaderThread::run()
{
}

char ThreadedLoader::LoaderThread::getStatus()
{
    char ret;

    _statusLock.lock();

    ret = _status;

    _statusLock.unlock();

    return ret;
}
