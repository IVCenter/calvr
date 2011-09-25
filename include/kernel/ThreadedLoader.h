/**
 * @file ThreadedLoader.h
 */
#ifndef CALVR_THREADED_LOADER_H
#define CALVR_THREADED_LOADER_H

#include <kernel/Export.h>
#include <kernel/CalVR.h>

#include <osgDB/ReadFile>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

#include <string>
#include <map>
#include <vector>

namespace cvr
{

/**
 * @brief Class that allows loading of files using a background thread and allows job status to be queried.  
 * Progress is synced across the graphics cluster
 */
class CVRKERNEL_EXPORT ThreadedLoader
{
    friend class CalVR;
    public:
        /**
         * @brief get a static pointer to the instance of the class
         */
        static ThreadedLoader * instance();

        /**
         * @brief Open a model file with a background thread
         * @param filename name of file to open
         * @param options osg options structure to use with the readNodeFile call
         * @return job number for this operation
         */
        int readNodeFile(std::string filename, osgDB::ReaderWriter::Options * options = NULL);

        /**
         * @brief Open a list of model files with a background thread
         * @param filenames list of files to open
         * @param options list of osg options to use with the readNodeFile call, ignored if not present or NULL
         * @return job number for this operation
         */
        int readNodeFiles(std::vector<std::string> & filenames, std::vector<osgDB::ReaderWriter::Options *> & options);

        /**
         * @brief Open an image file with a background thread
         * @param filename name of file to open
         * @param options osg options structure to use with the readImageFile call
         * @return job number for this operation
         */
        int readImageFile(std::string & filename, osgDB::ReaderWriter::Options * options = NULL);

        /**
         * @brief Open a list of image files with a background thread
         * @param filenames list of files to open
         * @param options list of osg options to use with the readImageFile call, ignored if not present or NULL
         * @return job number for this operation
         */
        int readImageFiles(std::vector<std::string> & filenames, std::vector<osgDB::ReaderWriter::Options *> & options);

        /**
         * @brief Runs a system command in a background thread over all nodes
         * @param command command to execute
         * @return job number for this operation
         */
        int systemCommand(std::string command);

        /**
         * @brief Runs a system command in a background thread on the master node only
         * @param command command to execute
         * @return job number for this operation
         */
        int systemCommandMasterOnly(std::string command);

        /**
         * @brief Returns if there was an error executing the job
         *
         * Returns true if job does not exist
         */
        bool isError(int job);

        /**
         * @brief Returns if the job is complete
         *
         * Returns false if job does not exist
         */
        bool isDone(int job);

        /**
         * @brief Returns the jobs progress 0-1
         *
         * Note: This is the progress through a list of file, i.e. if 5 of 10 
         * files are done, progress will return 0.5.
         * If the job is in error or does not exist, -1 is returned
         */
        float getProgress(int job);

        /**
         * @brief Removes a job, finished or not, from the threaded loader
         */
        void remove(int job);

        /**
         * @brief Get the result of a job that read a model file
         * @param job job number
         * @param node pointer to be set to the read model file node
         */
        void getNodeFile(int job, osg::ref_ptr<osg::Node> & node);

        /**
         * @brief Get the result of a job that read a list of model files
         * @param job job number
         * @param nodeList list to be filled with the read model file nodes
         */
        void getNodeFiles(int job, std::vector<osg::ref_ptr<osg::Node> > & nodeList);

        /**
         * @brief Get the result of a job that read an image file
         * @param job job number
         * @param image pointer to be set to the read image
         */
        void getImageFile(int job, osg::ref_ptr<osg::Image> & image);

        /**
         * @brief Get the result of a job that read a list of image files
         * @param job job number
         * @param imageList list to be filled with the read image files
         */
        void getImageFiles(int job, std::vector<osg::ref_ptr<osg::Image> > & imageList);

    protected:
        ThreadedLoader();
        virtual ~ThreadedLoader();
        void update();

        static ThreadedLoader * _myPtr; ///< static self pointer

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
            JobType type;
            int ret;
            std::vector<void*> ptrs;
        };

        class LoaderThread : public OpenThreads::Thread
        {
            public:
                LoaderThread(ThreadedJob * job);
                virtual ~LoaderThread();
                virtual void run();

                void quit();

                char getStatus();
                ThreadResult * getResult() { return &_result; }

            protected:
                ThreadedJob * _job;
                ThreadResult _result;
                char _status;

                bool _quit;
                OpenThreads::Mutex _quitLock;

                OpenThreads::Mutex _statusLock;
        };

        int _jobID;

        std::map<int,ThreadedJob*> _jobs;
        std::map<int,LoaderThread*> _threads;
        std::map<int,char> _jobStatus;
};

}

#endif
