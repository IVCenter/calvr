FIND_PATH(OASCLIENT_INCLUDE_DIR OASSound.h
  PATHS
  /home/calvr/CalVR/extern_libs/include
  /home/covise/covise/extern_libs/include
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OASCLIENT_LIBRARY 
  NAMES oasclient liboasclient
  PATHS
    /home/calvr/CalVR/extern_libs
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

SET(OASCLIENT_FOUND "NO")
IF(OASCLIENT_LIBRARY AND OASCLIENT_INCLUDE_DIR)
  SET(OASCLIENT_FOUND "YES")
ENDIF(OASCLIENT_LIBRARY AND OASCLIENT_INCLUDE_DIR)

