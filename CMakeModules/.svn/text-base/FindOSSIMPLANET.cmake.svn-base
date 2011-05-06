FIND_PATH(OSSIMPLANET_INCLUDE_DIR ossimplanet.h
  PATHS
  $ENV{OSSIMPLANET_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(OSSIMPLANET_INCLUDE_DIR ossimplanet.h
  PATHS
  /home/covise/covise/extern_libs/src/ossim/ossimPlanet/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OSSIMPLANET_LIBRARY 
  NAMES ossimPlanet
  PATHS $ENV{OSSIMPLANET_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib Linux64 src/ossimPlanet/Linux64.Opt
)
FIND_LIBRARY(OSSIMPLANET_LIBRARY 
  NAMES ossimPlanet
  PATHS
    /home/covise/covise/extern_libs/src/ossim/ossimPlanet
    /home/covise/covise/extern_libs
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib64 lib Linux64
)

SET(OSSIMPLANET_FOUND "NO")
IF(OSSIMPLANET_LIBRARY AND OSSIMPLANET_INCLUDE_DIR)
  SET(OSSIMPLANET_FOUND "YES")
ENDIF(OSSIMPLANET_LIBRARY AND OSSIMPLANET_INCLUDE_DIR)

