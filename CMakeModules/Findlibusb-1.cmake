FIND_PATH(LIBUSB1_INCLUDE_DIR libusb-1.0/libusb.h
  PATHS
  $ENV{LIBUSB1_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(LIBUSB1_INCLUDE_DIR libusb-1.0/libusb.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(LIBUSB1_LIBRARY 
  NAMES usb-1.0
  PATHS $ENV{LIBUSB1_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)
FIND_LIBRARY(LIBUSB1_LIBRARY 
  NAMES usb-1.0
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

SET(LIBUSB1_FOUND "NO")
IF(LIBUSB1_LIBRARY AND LIBUSB1_INCLUDE_DIR)
  SET(LIBUSB1_FOUND "YES")
ENDIF(LIBUSB1_LIBRARY AND LIBUSB1_INCLUDE_DIR)

