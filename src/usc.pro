TEMPLATE = app
TARGET = usc
DESTDIR = ../bin
MOC_DIR = tmp
UI_DIR = ui
OBJECTS_DIR = tmp
RCC_DIR = tmp
QT += core \
    gui
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
    QCMainWindow.h
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
	song/QUSongLine.cpp
FORMS += mediaplayer/QUMediaPlayer.ui \
    support/QUMessageBox.ui \
    support/QUProgressDialog.ui \
    QCMainWindow.ui
RESOURCES += resources/usc.qrc
TRANSLATIONS = resources/usc.de.ts
INCLUDEPATH += . \
	../include/taglib \
	../include/bass \
    song \
    mediaplayer \
    support \
    ui
LIBS += -L"../lib" \
	-ltag \
	-lbass
CONFIG -= debug-and-release release
CONFIG += debug
