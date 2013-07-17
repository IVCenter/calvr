FIND_PATH(CDBAPP_INCLUDE_DIR cdbapp/ApplicationDatabaseInterface.h
  PATHS
  $ENV{CDBAPP_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(CDBAPP_INCLUDE_DIR cdbapp/ApplicationDatabaseInterface.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(CDBAPP_LIBRARY 
  NAMES cdbapp
  PATHS $ENV{CDBAPP_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(CDBAPP_LIBRARY 
  NAMES cdbapp
  PATHS
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware
  PATH_SUFFIXES lib64 lib
)

SET(CDBAPP_FOUND "NO")
IF(CDBAPP_LIBRARY AND CDBAPP_INCLUDE_DIR)
  SET(CDBAPP_FOUND "YES")
ENDIF(CDBAPP_LIBRARY AND CDBAPP_INCLUDE_DIR)
