cmake_minimum_required(VERSION 3.20)
project(DesktopPet VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

set(SOURCES
    App/WinMain.cpp
    App/AppController.cpp
    Window/OverlayWindow.cpp
    Window/PetRender.cpp
    Playback/AnimationPlayer.cpp
    Playback/GIFDecoder.cpp
    Playback/APNGDecoder.cpp
    Playback/PNGSequenceDecoder.cpp
    Playback/VideoPlayer.cpp
    TrayIcon/TrayController.cpp
    Settings/AppSettings.cpp
    Settings/SettingsDialog.cpp
    Utilities/PlaceholderAnimation.cpp
)

set(HEADERS
    App/AppController.h
    Window/OverlayWindow.h
    Window/PetRender.h
    Playback/AnimationPlayer.h
    Playback/FrameSequence.h
    Playback/GIFDecoder.h
    Playback/APNGDecoder.h
    Playback/PNGSequenceDecoder.h
    Playback/VideoPlayer.h
    TrayIcon/TrayController.h
    Settings/AppSettings.h
    Settings/SettingsDialog.h
    Utilities/PlaceholderAnimation.h
    Resource.h
)

add_executable(DesktopPet WIN32 ${SOURCES} ${HEADERS})

target_include_directories(DesktopPet PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(DesktopPet PRIVATE
    user32
    gdi32
    gdiplus
    ole32
    oleaut32
    shell32
    comctl32
    shlwapi
    windowscodecs
    mfplat
    mfuuid
    mfreadwrite
    strmiids
    dwmapi
    msimg32
    uuid
    advapi32
    comdlg32
    shcore
)

target_compile_definitions(DesktopPet PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0A00
)

if(MSVC)
    target_compile_options(DesktopPet PRIVATE /W4 /permissive-)
endif()

if(MINGW)
    # windres must output COFF object files (.o) so GCC can link them.
    # Microsoft binary .res format is not accepted by the MinGW linker.
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/manifest.o
        COMMAND ${CMAKE_RC_COMPILER} -i ${CMAKE_CURRENT_SOURCE_DIR}/manifest.rc
            -o ${CMAKE_CURRENT_BINARY_DIR}/manifest.o
            --include-dir ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/manifest.rc
                ${CMAKE_CURRENT_SOURCE_DIR}/App.manifest)
    target_sources(DesktopPet PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/manifest.o)

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.o
        COMMAND ${CMAKE_RC_COMPILER} -i ${CMAKE_CURRENT_SOURCE_DIR}/Resource.rc
            -o ${CMAKE_CURRENT_BINARY_DIR}/resource.o
            --include-dir ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Resource.rc
                ${CMAKE_CURRENT_SOURCE_DIR}/Resource.h)
    target_sources(DesktopPet PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resource.o)

    option(INCLUDE_ICON "Embed application icon from app.ico" OFF)
    if(INCLUDE_ICON)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
            COMMAND ${CMAKE_RC_COMPILER} -i ${CMAKE_CURRENT_SOURCE_DIR}/icon.rc
                -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o
                --include-dir ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icon.rc
                    ${CMAKE_CURRENT_SOURCE_DIR}/app.ico)
        target_sources(DesktopPet PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
    endif()
endif()
