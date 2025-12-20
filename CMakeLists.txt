cmake_minimum_required(VERSION 3.16)

project(DwarfController VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets WebSockets Sql)

# Find Protobuf
find_package(Protobuf REQUIRED)

# Protobuf generation
file(GLOB PROTO_FILES "src/proto/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src)
include_directories(src/proto)
include_directories(${Protobuf_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/net/DwarfWebSocketClient.cpp
    src/net/DwarfWebSocketClient.h
    src/net/DwarfFinder.cpp
    src/net/DwarfFinder.h
    src/net/ProtobufHelper.cpp
    src/net/ProtobufHelper.h
    src/net/DwarfHttpClient.cpp
    src/net/DwarfHttpClient.h
    src/net/DwarfMessageDispatcher.cpp
    src/net/DwarfMessageDispatcher.h
    src/net/DwarfCameraController.cpp
    src/net/DwarfCameraController.h
    src/net/DwarfMotorController.cpp
    src/net/DwarfMotorController.h
    src/net/DwarfFocusController.cpp
    src/net/DwarfFocusController.h
    src/net/DwarfAstroController.cpp
    src/net/DwarfAstroController.h
    src/net/Lx200Server.cpp
    src/net/Lx200Server.h
    src/net/DwarfMjpegStream.cpp
    src/net/DwarfMjpegStream.h
    src/net/DwarfMjpegView.cpp
    src/net/DwarfMjpegView.h
    src/net/DwarfFtpDownloader.cpp
    src/net/DwarfFtpDownloader.h
    src/net/DwarfFtpClient.cpp
    src/net/DwarfFtpClient.h
    src/ui/MediaLightbox.cpp
    src/ui/MediaLightbox.h
    src/ui/CameraSettingsPanel.cpp
    src/ui/CameraSettingsPanel.h
    src/ui/StarMapWidget.cpp
    src/ui/StarMapWidget.h
    src/ui/AstroNavigationPanel.cpp
    src/ui/AstroNavigationPanel.h
    src/ui/ImageSettingsWidget.cpp
    src/ui/ImageSettingsWidget.h
    src/ui/VirtualJoystick.cpp
    src/ui/VirtualJoystick.h
    src/ui/MotorControlPanel.cpp
    src/ui/MotorControlPanel.h
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    resources.qrc
)

# Executable
add_executable(DwarfController ${SOURCES})

# Link libraries
target_link_libraries(DwarfController PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::WebSockets
    Qt6::Sql
    ${Protobuf_LIBRARIES}
)

set(I18N_SOURCE_DIR ${CMAKE_SOURCE_DIR}/i18n)
set(I18N_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/i18n)

add_custom_command(TARGET DwarfController POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${I18N_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${I18N_SOURCE_DIR}/DwarfController_de.qm
          ${I18N_SOURCE_DIR}/DwarfController_en.qm
          ${I18N_BINARY_DIR}
)

set(STYLES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/styles)
set(STYLES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/styles)

add_custom_command(TARGET DwarfController POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${STYLES_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${STYLES_SOURCE_DIR}/app.qss
          ${STYLES_BINARY_DIR}/app.qss
)

# Qt specific settings
set_target_properties(DwarfController PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
