TEMPLATE = app
UI_DIR = ui

QT += core \
	gui \
	widgets \
	webkit \
	network

#CONFIG += debug

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
	QUMainWindow.h \
	QUAboutDialog.h \
	support/QUMessageBox.h \
	support/QULogService.h \
	ribbon/QURibbonBar.h \
	QUMonty.h \
	monty/QUMontyArea.h \
	QUSongSupport.h \
	song/QUSongLine.h \
	song/QUSongInterface.h \
	song/QUSongFile.h \
	song/QUSongDatabase.h \
	QUStringSupport.h

SOURCES += main.cpp \
	QUMainWindow.cpp \
	QU.cpp \
	QUAboutDialog.cpp \
	support/QUMessageBox.cpp \
	support/QULogService.cpp \
	ribbon/QURibbonBar.cpp \
	QUMonty.cpp \
	monty/QUMontyArea.cpp \
	QUSongSupport.cpp \
	song/QUSongLine.cpp \
	song/QUSongFile.cpp \
	song/QUSongDatabase.cpp \
	QUStringSupport.cpp

FORMS += QUMainWindow.ui \
	QUAboutDialog.ui \
	support/QUMessageBox.ui \
	ribbon/QURibbonBar.ui \
	monty/QUMontyArea.ui

RESOURCES += resources/usc.qrc

TRANSLATIONS = resources/usc.de.ts \
	resources/usc.es.ts \
	resources/usc.fr.ts \
	resources/usc.pl.ts \
	resources/usc.pt.ts

INCLUDEPATH += . \
	monty \
	ribbon \
	song \
	support \
	ui

win32 {
	RC_FILE = usc.rc
	INCLUDEPATH += ../include/taglib \
		../include/taglib/toolkit \
		../include/bass \
		../include/bass_fx
	LIBS += -L"../lib" \
		-ltag \
		-lbass \
		-lbass_fx
}

macx {
	#ICON = images/app.icns
	#INCLUDEPATH += ../include/taglib \
	#    ../include/bass \
	#    ../include/bass_fx
	#LIBS += -L"../lib" \
	#    -ltag \
	#    -lbass \
	#    -lbass_fx
	#QMAKE_INFO_PLIST = min.us.Info.plist
}

unix:!macx {
	INCLUDEPATH += ../include/taglib \
		../include/taglib/toolkit \
		../include/bass \
		../include/bass_fx
	LIBS += -L"../lib" \
		-ltag64 \
		-lbass64 \
		-lbass_fx64
}

win32 {
QMAKE_EXTRA_TARGETS += revtarget
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = @echo \
	"const char *revision = \"r$(shell svnversion .)\"; const char *date_time = \"$(shell date /T)$(shell time /T)\";" > $$revtarget.target
revtarget.depends = $$SOURCES \
	$$HEADERS \
	$$FORMS
}

unix {
QMAKE_EXTRA_TARGETS += revtarget
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = @echo \"const char *revision = \\\"r$(shell svnversion .)\\\"; const char *date_time = \\\"$(shell date +%d.%m.%Y%6R)\\\";\" > $$revtarget.target
revtarget.depends = $$SOURCES \
	$$HEADERS \
	$$FORMS
}
