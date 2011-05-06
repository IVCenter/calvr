SET(LIB_NAME osgcaveui)
SET(HEADER_PATH ${CalVR_SOURCE_DIR}/include/${LIB_NAME} )
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Arrow.h
    ${HEADER_PATH}/Bar.h
    ${HEADER_PATH}/Button.h
    ${HEADER_PATH}/Calculator.h
    ${HEADER_PATH}/Card.h
    ${HEADER_PATH}/CheckBox.h
    ${HEADER_PATH}/ColorWheel.h
    ${HEADER_PATH}/CUI.h
    ${HEADER_PATH}/Dial.h
    ${HEADER_PATH}/DigitLabel.h
    ${HEADER_PATH}/DigitPanel.h
    ${HEADER_PATH}/Events.h
    ${HEADER_PATH}/FileBrowser.h
    ${HEADER_PATH}/FloatOMeter.h
    ${HEADER_PATH}/HeightFieldPickBox.h
    ${HEADER_PATH}/InputDevice.h
    ${HEADER_PATH}/Interaction.h
    ${HEADER_PATH}/Interface.h
    ${HEADER_PATH}/LogFile.h
    ${HEADER_PATH}/Marker.h
    ${HEADER_PATH}/Measure.h
    ${HEADER_PATH}/Message.h
    ${HEADER_PATH}/Paintbrush.h
    ${HEADER_PATH}/Panel.h
    ${HEADER_PATH}/PickBox.h
    ${HEADER_PATH}/RadioButton.h
    ${HEADER_PATH}/RadioGroup.h
    ${HEADER_PATH}/Rectangle.h
    ${HEADER_PATH}/TextureWidget.h
    ${HEADER_PATH}/Widget.h
    ${HEADER_PATH}/WidgetInfo.h
    ${HEADER_PATH}/VirvoDeps.h
    ${HEADER_PATH}/cvrvvtoolshed.h
)

ADD_LIBRARY(${LIB_NAME}
    ${CALVR_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Arrow.cpp
    Bar.cpp
    Button.cpp
    Calculator.cpp
    Card.cpp
    CheckBox.cpp
    ColorWheel.cpp
    CUI.cpp
    Dial.cpp
    DigitLabel.cpp
    DigitPanel.cpp
    FileBrowser.cpp
    FloatOMeter.cpp
    HeightFieldPickBox.cpp
    InputDevice.cpp
    Interaction.cpp
    Interface.cpp
    LogFile.cpp
    Marker.cpp
    Measure.cpp
    Message.cpp
    Paintbrush.cpp
    Panel.cpp
    PickBox.cpp
    RadioButton.cpp
    RadioGroup.cpp
    Rectangle.cpp
    TextureWidget.cpp
    Widget.cpp
    WidgetInfo.cpp
    VirvoDeps.cpp
    cvrvvtoolshed.cpp
)

#FIND_PACKAGE(Virvo)

INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(${VIRVO_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CalVR_SOURCE_DIR}/include/${LIB_NAME})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${OSG_LIBRARIES})
#TARGET_LINK_LIBRARIES(${LIB_NAME} ${VIRVO_LIBRARY})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib)
