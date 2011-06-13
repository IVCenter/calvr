IF(WIN32)
	IF(DYNAMIC_LINKING)
		ADD_DEFINITIONS(-DCVRMENU_LIBRARY)
	ELSE(DYNAMIC_LINKING)
		ADD_DEFINITIONS(-DCVRMENU_LIBRARY_STATIC)
	ENDIF(DYNAMIC_LINKING)
ENDIF(WIN32)

SET(LIB_NAME menu)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/MenuItem.h
    ${HEADER_PATH}/SubMenu.h
    ${HEADER_PATH}/MenuButton.h
    ${HEADER_PATH}/MenuCheckbox.h
    ${HEADER_PATH}/MenuRangeValue.h
    ${HEADER_PATH}/MenuText.h
    ${HEADER_PATH}/MenuTextButtonSet.h
    ${HEADER_PATH}/MenuBase.h
    ${HEADER_PATH}/PopupMenuBase.h
    ${HEADER_PATH}/BoardMenu.h
    ${HEADER_PATH}/MenuSystem.h
    ${HEADER_PATH}/MenuManager.h
    ${HEADER_PATH}/PopupMenu.h
    ${HEADER_PATH}/TabbedDialogPanel.h
    ${HEADER_PATH}/BoardPopupMenu.h
    ${HEADER_PATH}/BoardMenu/BoardMenuGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuButtonGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuCheckboxGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuRangeValueGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuTextGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuTextButtonSetGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuSubMenuGeometry.h
    ${HEADER_PATH}/Export.h
)

SET(LIB_SRC_FILES
    MenuItem.cpp
    SubMenu.cpp
    MenuButton.cpp
    MenuCheckbox.cpp
    MenuRangeValue.cpp
    MenuText.cpp
    MenuTextButtonSet.cpp
    MenuSystem.cpp
    BoardMenu.cpp
    MenuManager.cpp
    PopupMenu.cpp
    TabbedDialogPanel.cpp
    BoardPopupMenu.cpp
    BoardMenu/BoardMenuGeometry.cpp
    BoardMenu/BoardMenuButtonGeometry.cpp
    BoardMenu/BoardMenuCheckboxGeometry.cpp
    BoardMenu/BoardMenuRangeValueGeometry.cpp
    BoardMenu/BoardMenuTextGeometry.cpp
    BoardMenu/BoardMenuTextButtonSetGeometry.cpp
    BoardMenu/BoardMenuSubMenuGeometry.cpp
)

SET(LIB_EXTERNAL_INCLUDES
    ${OSG_INCLUDE_DIR}
)

SET(LIB_EXTERNAL_LIBRARIES
    ${OSG_LIBRARIES}
)

SET(LIB_INTERNAL_LIBRARIES
    config
    util
	input
)

SETUP_CORE_LIBRARY(${LIB_NAME})
