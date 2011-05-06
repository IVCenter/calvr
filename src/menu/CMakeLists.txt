SET(LIB_NAME menu)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/MenuItem.h
    ${HEADER_PATH}/SubMenu.h
    ${HEADER_PATH}/MenuButton.h
    ${HEADER_PATH}/MenuCheckbox.h
    ${HEADER_PATH}/MenuRangeValue.h
    ${HEADER_PATH}/MenuText.h
    ${HEADER_PATH}/MenuBase.h
    ${HEADER_PATH}/PopupMenuBase.h
    ${HEADER_PATH}/BoardMenu.h
    ${HEADER_PATH}/MenuSystem.h
    ${HEADER_PATH}/MenuManager.h
    ${HEADER_PATH}/PopupMenu.h
    ${HEADER_PATH}/BoardPopupMenu.h
    ${HEADER_PATH}/BoardMenu/BoardMenuGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuButtonGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuCheckboxGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuRangeValueGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuTextGeometry.h
    ${HEADER_PATH}/BoardMenu/BoardMenuSubMenuGeometry.h
)

ADD_LIBRARY(${LIB_NAME}
    ${CALVR_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    MenuItem.cpp
    SubMenu.cpp
    MenuButton.cpp
    MenuCheckbox.cpp
    MenuRangeValue.cpp
    MenuText.cpp
    MenuSystem.cpp
    BoardMenu.cpp
    MenuManager.cpp
    PopupMenu.cpp
    BoardPopupMenu.cpp
    BoardMenu/BoardMenuGeometry.cpp
    BoardMenu/BoardMenuButtonGeometry.cpp
    BoardMenu/BoardMenuCheckboxGeometry.cpp
    BoardMenu/BoardMenuRangeValueGeometry.cpp
    BoardMenu/BoardMenuTextGeometry.cpp
    BoardMenu/BoardMenuSubMenuGeometry.cpp
)

INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${OSG_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib)
