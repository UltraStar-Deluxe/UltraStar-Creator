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
    QULogService.h \
    QUMessageBox.h \
    QCMainWindow.h \
    QUAboutDialog.h

SOURCES += main.cpp \
    QCMainWindow.cpp \
    QU.cpp \
    QULogService.cpp \
    QUMessageBox.cpp \
    QUAboutDialog.cpp

FORMS += QUMessageBox.ui \
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

macx {
    #example
    #ICON = images/app.icns
    #OBJECTIVE_SOURCES += utils.mm
    #LIBS += -framework Cocoa
    #QMAKE_INFO_PLIST = min.us.Info.plist
    #
    #INCLUDEPATH += ../include/taglib \
    #    ../include/bass \
    #    ../include/bass_fx
    #LIBS += -L"../lib" \
    #    -ltag \
    #    -lbass \
    #    -lbass_fx
}

unix:!macx {
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
