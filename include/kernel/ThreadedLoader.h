#ifndef CALVR_THREADED_LOADER_H
#define CALVR_THREADED_LOADER_H

#include <kernel/CalVR.h>

#include <osgDB/ReadFile>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include <string>
#include <map>
#include <vector>

namespace cvr
{

class ThreadedLoader
{
    friend class CalVR;
    public:
        static ThreadedLoader * instance();

        int readNodeFile(std::string & filename, osgDB::ReaderWriter::Options * options = NULL);
        int readNodeFiles(std::vector<std::string> & filenames, std::vector<osgDB::ReaderWriter::Options *> & options);
        int readImageFile(std::string & filename, osgDB::ReaderWriter::Options * options = NULL);
        int readImageFiles(std::vector<std::string> & filenames, std::vector<osgDB::ReaderWriter::Options *> & options);
        int systemCommand(std::string command);
        int systemCommandMasterOnly(std::string command);

        bool isError(int job);
        bool isDone(int job);
        float getProgress(int job);
        void remove(int job);

        void getNodeFile(osg::Node * & node);
        void getNodeFiles(std::vector<osg::Node*> & nodeList);
        void getImageFile(osg::Image * & image);
        void getImageFiles(std::vector<osg::Image*> & imageList);

    protected:
        ThreadedLoader();
        virtual ~ThreadedLoader();
        void update();

        static ThreadedLoader * _myPtr;

        enum JobType
        {
            READ_NODE = 0,
            READ_NODE_LIST,
            READ_IMAGE,
            READ_IMAGE_LIST,
            SYSTEM,
            SYSTEM_MASTER
        };

        struct ThreadedJob
        {
            JobType type;
            std::vector<std::string> strs;
            std::vector<osgDB::ReaderWriter::Options *> options;
        };

        struct ThreadResult
        {
            int ret;
            std::vector<void*> ptrs;
        };

        class LoaderThread : public OpenThreads::Thread
        {
            public:
                LoaderThread(ThreadedJob * job);
                virtual ~LoaderThread();
                virtual void run();

                char getStatus();
                ThreadResult * getResult() { return &_result; }

            protected:
                ThreadedJob * _job;
                ThreadResult _result;
                char _status;

                OpenThreads::Mutex _statusLock;
        };

        int _jobID;

        std::map<int,ThreadedJob*> _jobs;
        std::map<int,LoaderThread*> _threads;
        std::map<int,char> _jobStatus;
};

}

#endif
