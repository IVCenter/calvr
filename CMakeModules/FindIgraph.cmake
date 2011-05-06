FIND_PATH(IGRAPH_INCLUDE_DIR igraph/igraph.h
  PATHS
  $ENV{OSSIM_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(IGRAPH_INCLUDE_DIR igraph/igraph.h
  PATHS
  /home/covise/covise/extern_libs/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(IGRAPH_LIBRARY 
  NAMES igraph
  PATHS $ENV{IGRAPH_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(IGRAPH_LIBRARY 
  NAMES igraph
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

SET(IGRAPH_FOUND "NO")
IF(IGRAPH_LIBRARY AND IGRAPH_INCLUDE_DIR)
  SET(IGRAPH_FOUND "YES")
ENDIF(IGRAPH_LIBRARY AND IGRAPH_INCLUDE_DIR)

