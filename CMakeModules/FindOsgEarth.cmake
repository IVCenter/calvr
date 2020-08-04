FIND_PATH(OSGEARTH_INCLUDE_DIR osgEarth/Map
  PATHS
  $ENV{OSGEARTH_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(OSGEARTH_INCLUDE_DIR osgEarth/Map
  PATHS
  /home/covise/covise/extern_libs/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OSGEARTH_LIBRARY 
  NAMES osgEarth
  PATHS $ENV{OSGEARTH_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(OSGEARTH_LIBRARY 
  NAMES osgEarth
  PATHS
    /home/covise/covise/extern_libs
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib64 lib
)

SET(OSGEARTH_FOUND "NO")
IF(OSGEARTH_LIBRARY AND OSGEARTH_INCLUDE_DIR)
  SET(OSGEARTH_FOUND "YES")
ENDIF(OSGEARTH_LIBRARY AND OSGEARTH_INCLUDE_DIR)

