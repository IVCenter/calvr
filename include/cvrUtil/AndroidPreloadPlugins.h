/**
 * @file AndroidPreloadPlugins.h
 */

#ifndef CALVR_ANDROID_PRELOAD_PLUGINS
#define CALVR_ANDROID_PRELOAD_PLUGINS

#include <cvrKernel/CVRPlugin.h>
#include <map>

#define REGISTER(ClassName) \
    class Register##ClassName \
    { \
    public: \
        static cvr::CVRPlugin* instance() \
        { \
            return new ClassName; \
        } \
    private: \
        static const Register _staticRegister; \
    }; \
    const Register Register##ClassName::_staticRegister(#ClassName, Register##ClassName::instance());

namespace cvr {
    typedef std::map<const std::string, cvr::CVRPlugin *> create_obj_map;

    class LazySingleton {
    public:
        template<class T>
        static T &instance() {
            static T _instance;
            return _instance;
        }
    };

    class ClassFactory {

    public:
        static cvr::CVRPlugin *getInstance(const std::string &className);

        static void registerClass(const std::string &className, cvr::CVRPlugin *fp);

        static void initAll(const std::vector<std::string> nameStorage);
    };

    class Register {
    public:
        Register(const char *className, cvr::CVRPlugin *fp) {
            ClassFactory::registerClass(className, fp);
        }
    };
}

#endif
