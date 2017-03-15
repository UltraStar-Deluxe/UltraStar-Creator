TEMPLATE = app
UI_DIR = ui

QT += core \
	gui \
	widgets \
	network

#CONFIG += debug

CONFIG(release, debug|release) {
TARGET = UltraStarCreator
	DESTDIR = ../bin/UltraStarCreator
	MOC_DIR = tmp/release
	OBJECTS_DIR = tmp/release
	RCC_DIR = tmp/release
}

CONFIG(debug, debug|release) {
	TARGET = UltraStarCreator_debug
	DESTDIR = ../bin/UltraStarCreator_debug
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
INCLUDEPATH += ../include/taglib \
	../include/bass \
	../include/bass_fx
win32 {
	RC_FILE = usc.rc
	LIBS += -L"../lib/Windows" \
		-ltag \
		-lbass \
		-lbass_fx
}

macx {
	#ICON = images/app.icns
	LIBS += -L"../lib/MacOS" \
		-ltag \
		-lbass \
		-lbass_fx
	CONFIG += app_bundle
	#QMAKE_INFO_PLIST = min.us.Info.plist
}

unix:!macx {
	LIBS += -L"../lib/Unix" \
		-ltag \
		-lbass64 \
		-lbass_fx64
#		-ltag \
#		-lbass \
#		-lbass_fx
}

QMAKE_EXTRA_TARGETS += revtarget
PRE_TARGETDEPS += version.h
revtarget.target = version.h

win32 {
revtarget.commands = @echo \
	"const char *revision = \"r$(shell svnversion .)\"; const char *date_time = \"$(shell date /T)$(shell time /T)\";" > $$revtarget.target
}

unix {
revtarget.commands = @echo \
	"const char *revision = \\\"rev`git rev-parse --short HEAD`\\\"\\; \
	const char *date_time = \\\"`date`\\\"\\;" \
	> $${PWD}/$$revtarget.target
}

revtarget.depends = $$SOURCES \
	$$HEADERS \
	$$FORMS

mac {
	dylibs.files = ../lib/MacOS/libbass.dylib \
		../lib/MacOS/libbass_fx.dylib \
		../lib/MacOS/libtag.1.15.1.dylib
	dylibs.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += dylibs
	ICON = usc.icns
}
