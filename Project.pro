# Filament libs require the libc++ standard library
if(USE_CLANG) {
  message("Using the clang++ compiler")
  QMAKE_CXX = /usr/bin/clang++
  QMAKE_LINK = /usr/bin/clang++
  QMAKE_CXXFLAGS += -stdlib=libc++
} else {
  message("Using the g++ compiler")
  message("Forcing g++ to use libc++")
  include(force_gcc_libcxx.pri)
}

# We want to build an executable
TEMPLATE = app
# Specify where we should store our build files
BUILD_PATH = build
# Name our executable
TARGET = $${BUILD_PATH}/bin/QtFilamentPBR


# Specify where to find the ui forms
UI_HEADERS_DIR = ui
UI_DIR = ui
# Specify where to write our object files
OBJECTS_DIR = $${BUILD_PATH}/obj
MOC_DIR = $${BUILD_PATH}/moc

QT += opengl core gui
CONFIG += console c++14
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/ui \
    $$PWD/materials \
    /public/devel/2018/include \
    ${FILAMENT_PATH}/include 

HEADERS += $$files(include/*.h, true)
SOURCES += $$files(src/*.cpp, true)

FORMS += ui/applayout.ui

#LIBS += -lOpenImageIO
LIBS += -L${FILAMENT_PATH}/lib/x86_64
LIBS += \
  -lfilament \
  -lbluegl \
  -lbluevk \
  -lfilabridge \
  -lfilaflat \
  -lutils \
  -limage \
#  -lgeometry \
  -lsmol-v \
  -lassimp \
  -lfilameshio \
  -lmeshoptimizer \
  -ldl \
  -pthread 

# Need this to find metal symbols
macx:{
    QMAKE_CXXFLAGS += -x objective-c++
    QMAKE_LFLAGS += -framework Metal -framework MetalKit -framework Cocoa -framework CoreFoundation -fobjc-link-runtime
}

QMAKE_CXXFLAGS += -Ofast -msse -msse2 -msse3 -march=native -ffast-math -funroll-loops 
QMAKE_CXXFLAGS += -Wall -Wextra -fdiagnostics-color

