# UltraStar Creator Development README

[![Unix/OSX build status](https://travis-ci.org/UltraStar-Deluxe/UltraStar-Creator.svg?branch=master)](https://travis-ci.org/UltraStar-Deluxe/UltraStar-Creator)
[![Windows build status](https://ci.appveyor.com/api/projects/status/x1i5i0uqp2cf6xjv/branch/master?svg=true)](https://ci.appveyor.com/project/bohning/ultrastar-creator/branch/master)
[![License](https://img.shields.io/badge/license-GPLv2-blue.svg)](LICENSE)

![UltraStar Creator Logo](https://github.com/UltraStar-Deluxe/UltraStar-Creator/blob/master/src/resources/icons/bean.png)


### 1. About
UltraStar Creator (usc) is a free and open source song creation tool using Qt. It should be compilable and runnable on Windows, Linux and Mac OS.

### 2. Release Notes
(under construction)

### 3. Command-Line Parameters
(under construction)

### 4. Controls
(under construction)

### 5. Build and Run
Download the Qt framework for your respective platform from https://www.qt.io/download-open-source/.

#### Compiling on Windows
(under construction)

1. Install Qt (MinGW).
2. Compile UltraStar Creator (disable shadow build).
3. Add DLL dependencies to /bin/wip directory (Qt + MinGW runtime libraries, bass, bass_fx, taglib).
4. Start creating new songs!

#### Compiling on Linux
1. Install the Qt framework: `sudo apt get install qt5-default qttools5-dev-tools qtbase5-dev-tools qt5-qmake`
2. Install dependencies: `sudo apt get install libtag1-dev`
3. Compile
   * from command line: `cd src && qmake usc.pro && make` 
   * using Qt Creator: open usc.pro in Qt Creator, disable shadow build in Project tab, compile and run
4. Start creating new songs!

#### Compiling on OS X
(under construction)

1. Install Qt (clang).
2. Compile UltraStar Creator (disable shadow build).
3. Start creating new songs!

### 6. Contribute
Feel free to fork this project, modify it to your hearts content and maybe also do pull requests to this repository for additional features, improvements or clean-ups.
