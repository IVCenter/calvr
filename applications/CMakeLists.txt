#If your app has dependencies, use this format
#OPTION(APPS_YOURNAMEHERE "Build yournamehere application" OFF)

#IF(APPS_YOURNAMEHERE)
#   ADD_SUBDIRECTORY(yournamehere)
#ENDIF(APPS_YOURNAMEHERE)

ADD_SUBDIRECTORY(CollabServer)
