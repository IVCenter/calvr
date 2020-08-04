INCLUDE_DIRECTORIES(${CALVR_INCLUDE_DIR})

#If your app has dependencies, use this format
#OPTION(APPS_YOURNAMEHERE "Build yournamehere application" OFF)

#IF(APPS_YOURNAMEHERE)
#   ADD_SUBDIRECTORY(yournamehere)
#ENDIF(APPS_YOURNAMEHERE)

OPTION(APPS_VRPN_LIBUSB "Build vrpn libusb server for gyromouse" OFF)

IF(APPS_VRPN_LIBUSB)
    ADD_SUBDIRECTORY(vrpn_libusb)
ENDIF(APPS_VRPN_LIBUSB)

OPTION(APPS_VRPN_LIBUSB_GENERAL "Build vrpn server that uses libusb" OFF)

IF(APPS_VRPN_LIBUSB_GENERAL)
    ADD_SUBDIRECTORY(vrpn_libusb_general)
ENDIF(APPS_VRPN_LIBUSB_GENERAL)

ADD_SUBDIRECTORY(CollabServer)

