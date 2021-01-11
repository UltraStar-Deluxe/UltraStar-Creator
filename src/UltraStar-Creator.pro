CONFIG += c++11
TEMPLATE = app
UI_DIR = ui

QT += core \
	gui \
	widgets \
	network

CONFIG(release, debug|release) {
TARGET = UltraStar-Creator
	DESTDIR = ../bin/release
	MOC_DIR = tmp/release
	OBJECTS_DIR = tmp/release
	RCC_DIR = tmp/release
}

CONFIG(debug, debug|release) {
	TARGET = UltraStar-Creator_debug
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
	monty/QUMontyArea.ui

RESOURCES += resources/UltraStar-Creator.qrc

TRANSLATIONS = resources/UltraStar-Creator.de.ts \
	resources/UltraStar-Creator.es.ts \
	resources/UltraStar-Creator.fr.ts \
	resources/UltraStar-Creator.pl.ts \
	resources/UltraStar-Creator.pt.ts

INCLUDEPATH += . \
	monty \
	song \
	support \
	ui

INCLUDEPATH += ../include/bass \
	../include/bass_fx \
	../include/cld2/public

win32 {
	INCLUDEPATH += ../include/taglib

	LIBS += -L"../lib/win64" \
		-ltag \
		-lbass \
		-lbass_fx \
		-lcld2

	RC_ICONS += UltraStar-Creator.ico
}

macx {
	LIBS += -L"../lib/macx" \
		-lbass \
		-lbass_fx \
		-lcld2

	CONFIG += link_pkgconfig
	PKGCONFIG += taglib

	CONFIG += app_bundle
	#QMAKE_RPATHDIR += @executable_path/../Frameworks
	QMAKE_LFLAGS += -Wl,-rpath,@executable_path/../Frameworks/
	#QMAKE_INFO_PLIST = min.us.Info.plist

	ICON = resources/UltraStar-Creator.icns
}

unix:!macx {
	LIBS += -L"../lib/unix" \
		-lbass \
		-lbass_fx

	CONFIG += link_pkgconfig
	PKGCONFIG += taglib

	QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/lib\''
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
	QMAKE_POST_LINK += $$sprintf($${QMAKE_MKDIR_CMD}, $$shell_path($${DESTDIR}/lib/)) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/unix/libbass.so) $$shell_path($${DESTDIR}/lib) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/unix/libbass_fx.so) $$shell_path($${DESTDIR}/lib) $$escape_expand(\\n\\t)
}

win32 {
	# Run windeployqt to bundle the required Qt libraries with the application
	CONFIG(release, debug|release) {
		QMAKE_POST_LINK += windeployqt64releaseonly --release --no-translations --no-system-d3d-compiler --compiler-runtime --no-angle --no-opengl-sw $$shell_path($${DESTDIR})/UltraStar-Creator.exe $$escape_expand(\\n\\t)
	}

	CONFIG(debug, debug|release) {
		QMAKE_POST_LINK += windeployqt --debug --no-translations --no-system-d3d-compiler --compiler-runtime --no-angle --no-opengl-sw $$shell_path($${DESTDIR})/UltraStar-Creator_debug.exe $$escape_expand(\\n\\t)
	}

	# Clean up after running windeployqt, removing some superfluous Qt libraries
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/Qt5Svg*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/iconengines/qsvgicon*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_DIR} $$shell_path($${DESTDIR}/iconengines) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qicns*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qico*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qsvg*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qtga*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qtiff*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qwbmp*.dll) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$shell_path($${DESTDIR}/imageformats/qwebp*.dll) $$escape_expand(\\n\\t)

	# Manually add bass, bass_fx, libtag and libcld2 libraries
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/bass.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/bass_fx.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/libtag.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/libcld2.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)

	# Copy SSL/TLS libraries
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/capi.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/dasync.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/libcrypto-1_1-x64.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../lib/win64/libssl-1_1-x64.dll) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)

	# Add syllabification dictionaries
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../syllabification/*.txt) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)

	# Manually add changes.txt
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path(../doc/changes.txt) $$shell_path($${DESTDIR}) $$escape_expand(\\n\\t)

	CONFIG(release, debug|release) {
		# Create a fancy Windows installer
		QMAKE_POST_LINK += $$shell_quote(C:\Program Files (x86)\NSIS\makensis.exe) $$shell_path(../setup/win64/UltraStar-Creator.nsi) $$escape_expand(\\n\\t)
	}
}

macx {
	dylibs.files = ../lib/macx/libbass.dylib \
		../lib/macx/libbass_fx.dylib \
		../lib/macx/libcld2.dylib
	dylibs.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += dylibs
	
	syllabification.files = $$files(../syllabification/*.txt)
	syllabification.path = Contents/Resources
	QMAKE_BUNDLE_DATA += syllabification

	# Run macdeployqt to bundle the required Qt libraries with the application
	QMAKE_POST_LINK += macdeployqt ../bin/release/UltraStar-Creator.app -libpath=../lib/macx $$escape_expand(\\n\\t)

	# Fix path to external libraries in app bundle
	QMAKE_POST_LINK += install_name_tool -change @loader_path/libbass.dylib @executable_path/../Frameworks/libbass.dylib ../bin/release/UltraStar-Creator.app/Contents/MacOS/UltraStar-Creator $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change @loader_path/libbass_fx.dylib @executable_path/../Frameworks/libbass_fx.dylib ../bin/release/UltraStar-Creator.app/Contents/MacOS/UltraStar-Creator $$escape_expand(\\n\\t)
	QMAKE_POST_LINK += install_name_tool -change @loader_path/libcld2.dylib @executable_path/../Frameworks/libcld2.dylib ../bin/release/UltraStar-Creator.app/Contents/MacOS/UltraStar-Creator $$escape_expand(\\n\\t)

	# Create a fancy Mac disk image
	QMAKE_POST_LINK += create-dmg --volname UltraStar-Creator --volicon resources/UltraStar-Creator.icns --app-drop-link 350 170 --background ../setup/macx/img/UltraStar-Creator_bg.png --hide-extension UltraStar-Creator.app --window-size 500 300 --text-size 14 --icon-size 64 --icon UltraStar-Creator.app 150 170 --no-internet-enable --skip-jenkins "../bin/release/UltraStar-Creator.dmg" ../bin/release/UltraStar-Creator.app/
}
