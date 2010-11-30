TEMPLATE = app
UI_DIR = ui

QT += core \
    gui \
    webkit \
    network

# CONFIG -= debug_and_release \
# release
CONFIG += debug
#CONFIG += release

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
    mediaplayer/QUAutoCue.h \
    mediaplayer/QUMediaPlayer.h \
    support/QULogService.h \
    support/QUMessageBox.h \
    support/QUProgressDialog.h \
    support/QUMetaphoneString.h \
    QCMainWindow.h \
    QUAboutDialog.h \
    QUProxyDialog.h

SOURCES += main.cpp \
    QCMainWindow.cpp \
    QU.cpp \
    QUStringSupport.cpp \
    QUSongSupport.cpp \
    mediaplayer/QUAutoCue.cpp \
    mediaplayer/QUMediaPlayer.cpp \
    support/QULogService.cpp \
    support/QUProgressDialog.cpp \
    support/QUMessageBox.cpp \
    support/QUMetaphoneString.cpp \
    song/QUSongFile.cpp \
    song/QUSongLine.cpp \
    QUAboutDialog.cpp \
    QUProxyDialog.cpp

FORMS += mediaplayer/QUMediaPlayer.ui \
    support/QUMessageBox.ui \
    support/QUProgressDialog.ui \
    QCMainWindow.ui \
    QUAboutDialog.ui \
    QUProxyDialog.ui
	
RESOURCES += resources/usc.qrc

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
revtarget.commands = $(shell del version.h ) \
                     $(shell SubWCRev . version.in version.h )
revtarget.depends = $$SOURCES \
    $$HEADERS \
    $$FORMS

