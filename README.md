# UltraStar-Creator Development README

[![Build](https://github.com/UltraStar-Deluxe/UltraStar-Creator/actions/workflows/main.yml/badge.svg)](https://github.com/UltraStar-Deluxe/UltraStar-Creator/actions/workflows/main.yml)
[![License](https://img.shields.io/badge/license-GPLv2-blue.svg)](LICENSE)

![UltraStar-Creator Logo](https://github.com/UltraStar-Deluxe/UltraStar-Creator/blob/master/src/resources/icons/UltraStar-Creator128.png)


### 1. About
UltraStar-Creator (usc) is a free and open source song creation tool using Qt. It should be compilable and runnable on Windows, Linux and Mac OS.

### 2. Release Notes
(under construction)

### 3. Command-Line Parameters
(under construction)

### 4. Controls
(under construction)

### 5. Build and Run
#### Compiling on Windows (MSVC)

Note: you might have to setup the MSVC commandline environment before building.
1. Install Qt6. We recommend Qt 6.5
2. Build taglib. From command line (in repository root):
```
git clone --recurse-submodules https://github.com/taglib/taglib.git
cd taglib
cmake -B build -G "Visual Studio 17 2022" -A x64 -DWITH_ZLIB=OFF -DBUILD_SHARED_LIBS=ON -DENABLE_STATIC_RUNTIME=OFF -DBUILD_TESTING=OFF
msbuild build/install.vcxproj -p:Configuration=Release
cp -r "C:/Program Files/taglib/include/taglib" ../include
copy build/taglib/Release/tag.dll ../lib/win64
copy build/taglib/Release/tag.lib ../lib/win64
```
3. Build cld2. Make sure you cloned with submodules included. From command line (in repository root):
```
cd include/cld2/internal
cl /TP /GR /EHsc /c /MD cldutil.cc cldutil_shared.cc compact_lang_det.cc compact_lang_det_hint_code.cc compact_lang_det_impl.cc  debug.cc fixunicodevalue.cc generated_entities.cc  generated_language.cc generated_ulscript.cc getonescriptspan.cc lang_script.cc offsetmap.cc  scoreonescriptspan.cc tote.cc utf8statetable.cc cld_generated_cjk_uni_prop_80.cc cld2_generated_cjk_compatible.cc cld_generated_cjk_delta_bi_32.cc generated_distinct_bi_0.cc cld2_generated_quad0122.cc cld2_generated_deltaocta0122.cc cld2_generated_distinctocta0122.cc cld_generated_score_quad_octa_0122.cc
link /dll /out:cld2.dll *.obj
lib /out:cld2.lib *.obj
copy .\cld2.lib ..\..\..\lib\win64
copy .\cld2.dll ..\..\..\lib\win64
```
4. Build UltraStar-Creator
   * from command line: `cd src && qmake6 UltraStar-Creator.pro -spec win32-msvc && nmake` 
   * using Qt Creator: open `src\UltraStar-Creator.pro` in Qt Creator, disable shadow build in Project tab, then build (CTRL+B)
5. Run `UltraStar-Creator.exe` in `bin\releease` and start creating new songs!

#### Compiling on Linux
These are example instructions for Ubuntu. Package names may have to be adapted for your distribution.

1. Install Qt6. We recommend Qt 6.5
2. Install dependencies: `sudo apt-get install libgl1-mesa-dev build-essential libtag1-dev libcld2-dev libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-xinerama0 libxcb-xkb-dev libxkbcommon-x11-0 libxcb-cursor0 libgtk2.0-dev libfuse2`
3. Build UltraStar-Creator. From repository root:
   * via command line (from repository root): `cd src && qmake6 UltraStar-Creator.pro && make` 
   * using Qt Creator: open `src/UltraStar-Creator.pro` in Qt Creator, disable shadow build in Project tab, then build (CTRL+B)
4. Run `UltraStar-Creator` in `bin/release` and start creating new songs!

#### Compiling on Mac OS X
We support both x86 (Intel) and ARM (M1/M2) Macs.

1. Install homebrew via `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
2. Install the Qt framework via `brew install qt`.
3. Install external dependencies via `brew install taglib`
4. Build cld2. Make sure you cloned with submodules included. From command line (in repository root):
```
cd include
patch cld2/internal/compile_libs.sh cld2-mac-compile.patch
cd cld2/internal/
export CFLAGS="-Wno-narrowing -O3"
./compile_libs.sh
cp libcld2.dylib ../../../lib/macx
```
5. Build UltraStar-Creator
   * from command line: `cd src && qmake UltraStar-Creator.pro && make` 
   * using Qt Creator: open `src/UltraStar-Creator.pro` in Qt Creator, disable shadow build in Project tab, then build (CTRL+B)
6. Run `UltraStar-Creator.app` in `bin/release` and start creating new songs!

### 6. Contribute
Feel free to fork this project, modify it to your hearts content and maybe also do pull requests to this repository for additional features, improvements or clean-ups.
