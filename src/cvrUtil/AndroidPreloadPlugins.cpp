#include <cvrUtil/AndroidPreloadPlugins.h>
using namespace cvr;

cvr::CVRPlugin *ClassFactory::getInstance(const std::string &className){
  return LazySingleton::instance<create_obj_map>()[className];
}

void ClassFactory::registerClass(const std::string &className, cvr::CVRPlugin* fp){
  LazySingleton::instance<create_obj_map>()[className] =fp;
//  nameStorage.push_back(className);
}

void ClassFactory::initAll(const std::vector<std::string> nameStorage){
  for(auto itr = nameStorage.begin(); itr<nameStorage.end(); itr++)
    LazySingleton::instance<create_obj_map>()[*itr]->init();
}
