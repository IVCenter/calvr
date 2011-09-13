FIND_PATH(MYSQL_INCLUDE_DIR mysql++/mysql++.h
  PATHS
  $ENV{MYSQL_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(MYSQL_INCLUDE_DIR mysql++/mysql++.h
  PATHS
  /home/covise/covise/extern_libs/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(MYSQL_LIBRARY 
  NAMES mysqlpp
  PATHS $ENV{MYSQL_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(MYSQL_LIBRARY 
  NAMES mysqlpp
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

SET(MYSQL_FOUND "NO")
IF(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIR)
  SET(MYSQL_FOUND "YES")
ENDIF(MYSQL_LIBRARY AND MYSQL_INCLUDE_DIR)

