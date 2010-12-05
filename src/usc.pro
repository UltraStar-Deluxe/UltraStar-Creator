TEMPLATE = app
UI_DIR = ui

QT += core \
    gui \
    webkit \
    network

# CONFIG -= debug_and_release \
# release
#CONFIG += debug
CONFIG += release

CONFIG(release, debug|release) {
        TARGET = usc
        DESTDIR = ../bin/usc
        MOC_DIR = tmp/release
        OBJECTS_DIR = tmp/release
        RCC_DIR = tmp/release
}

CONFIG(debug, debug|release) {
        TARGET = usc_debug
        DESTDIR = ../bin/usc_debug
        MOC_DIR = tmp/debug
        OBJECTS_DIR = tmp/debug
        RCC_DIR = tmp/debug
}

HEADERS += main.h \
    QU.h \
    QUStringSupport.h \
    QUSongSupport.h \
    song/QUSongInterface.h \
    song/QUSongFile.h \
    song/QUSongLine.h \
    support/QULogService.h \
    support/QUMessageBox.h \
    QCMainWindow.h \
    QUAboutDialog.h

SOURCES += main.cpp \
    QCMainWindow.cpp \
    QU.cpp \
    QUStringSupport.cpp \
    QUSongSupport.cpp \
    support/QULogService.cpp \
    support/QUMessageBox.cpp \
    song/QUSongFile.cpp \
    song/QUSongLine.cpp \
    QUAboutDialog.cpp

FORMS += support/QUMessageBox.ui \
    QCMainWindow.ui \
    QUAboutDialog.ui
	
RESOURCES += \
    resources/usc.qrc

TRANSLATIONS = resources/usc.de.ts \
    resources/usc.es.ts \
    resources/usc.fr.ts \
    resources/usc.it.ts \
    resources/usc.pl.ts

INCLUDEPATH += . \
    song \
    mediaplayer \
    support \
    ui

win32 { 
    RC_FILE = usc.rc
    INCLUDEPATH += ../include/taglib \
        ../include/bass \
        ../include/bass_fx
    LIBS += -L"../lib" \
        -ltag \
        -lbass \
        -lbass_fx
}
unix { 
    INCLUDEPATH += /usr/include/taglib \
        /usr/local/include/libbass
    LIBS += -L"/usr/local/lib/libbass" \
        -ltag \
        -lbass \
        -lbass_fx
}

QMAKE_EXTRA_TARGETS += revtarget
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = $(shell SubWCRev . version.in version.h )
revtarget.depends = $$SOURCES \
    $$HEADERS \
    $$FORMS
