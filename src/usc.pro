TEMPLATE = app
UI_DIR = ui

QT += core \
	gui \
	widgets \
	network

CONFIG(release, debug|release) {
TARGET = UltraStarCreator
	DESTDIR = ../bin/release
	MOC_DIR = tmp/release
	OBJECTS_DIR = tmp/release
	RCC_DIR = tmp/release
}

CONFIG(debug, debug|release) {
	TARGET = UltraStarCreator_debug
	DESTDIR = ../bin/debug
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

INCLUDEPATH += ../include/bass \
	../include/bass_fx

win32 {
	INCLUDEPATH += ../include/taglib \

	LIBS += -L"../lib/win32" \
		-ltag \
		-lbass \
		-lbass_fx

	RC_FILE = usc.rc
}

macx {
	LIBS += -L"../lib/macx" \
		-lbass \
		-lbass_fx

	CONFIG += link_pkgconfig
	PKGCONFIG += taglib

	CONFIG += app_bundle
	QMAKE_RPATHDIR += @executable_path/../Frameworks
	#QMAKE_INFO_PLIST = min.us.Info.plist

	ICON = resources/usc.icns
}

unix:!macx {
	LIBS += -L"../lib/unix" \
		-lbass \
		-lbass_fx

	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
}

QMAKE_EXTRA_TARGETS += revtarget
PRE_TARGETDEPS += version.h
revtarget.target = version.h

win32 {
	revtarget.commands = "$$system(echo 'const char *revision = \"$$system(git describe --always)\";' > $$revtarget.target)"
	revtarget.commands += "$$system(echo 'const char *date_time = \"$$system(date /T) $$system(time /T)\";' >> $$revtarget.target)"
}

unix {
	revtarget.commands = @echo "const char *revision = \\\"`git describe --always`\\\"\\; const char *date_time = \\\"`date +'%d.%m.%Y %H:%M'`\\\"\\;" > $${PWD}/$$revtarget.target
}

revtarget.depends = $$SOURCES \
	$$HEADERS \
	$$FORMS

unix:!macx {
	QMAKE_POST_LINK += $${QMAKE_COPY} $$IN_PWD/../lib/unix/libbass.so $$IN_PWD/../bin/release $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$IN_PWD/../lib/unix/libbass_fx.so $$IN_PWD/../bin/release $$escape_expand(\\n\\t)
}

win32 {
	QMAKE_POST_LINK += windeployqt --release --no-translations --no-system-d3d-compiler --compiler-runtime --no-angle --no-opengl-sw ..\bin\release\UltraStarCreator.exe $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/Qt5Svg.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/iconengines/qsvgicon.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_DIR} $$shell_path($${DESTDIR}/iconengines) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qicns.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qico.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qsvg.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qtga.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qtiff.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qwbmp.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qwebp.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win32/bass.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win32/bass_fx.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win32/libtag.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../doc/changes.txt) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	#QMAKE_POST_LINK += $$shell_quote(C:\Program Files (x86)\NSIS\makensis.exe) $$shell_path(../setup/win32/usc.nsi) $$escape_expand(\\n\\t)
}

macx {
	dylibs.files = ../lib/macx/libbass.dylib \
		../lib/macx/libbass_fx.dylib
	dylibs.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += dylibs

	QMAKE_POST_LINK += macdeployqt ../bin/release/UltraStarCreator.app $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change @loader_path/libbass.dylib @executable_path/../Frameworks/libbass.dylib ../bin/release/UltraStarCreator.app/Contents/MacOS/UltraStarCreator $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change @loader_path/libbass_fx.dylib @executable_path/../Frameworks/libbass_fx.dylib ../bin/release/UltraStarCreator.app/Contents/MacOS/UltraStarCreator $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui ../bin/release/UltraStarCreator.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore ../bin/release/UltraStarCreator.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore ../bin/release/UltraStarCreator.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore ../bin/release/UltraStarCreator.app/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui ../bin/release/UltraStarCreator.app/Contents/PlugIns/platforms/libqcocoa.dylib $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore ../bin/release/UltraStarCreator.app/Contents/PlugIns/platforms/libqcocoa.dylib $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport @executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport ../bin/release/UltraStarCreator.app/Contents/PlugIns/platforms/libqcocoa.dylib $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change /usr/local/Cellar/qt5/5.8.0_1/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets ../bin/release/UltraStarCreator.app/Contents/PlugIns/platforms/libqcocoa.dylib $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += ../setup/macx/create-dmg --volname UltraStarCreator --volicon resources/usc.icns --app-drop-link 350 170 --background ../setup/macx/img/usc_bg.png --hide-extension UltraStarCreator.app --window-size 500 300 --text-size 14 --icon-size 64 --icon UltraStarCreator.app 150 170 "../bin/release/UltraStarCreator.dmg" ../bin/release/UltraStarCreator.app/
}
