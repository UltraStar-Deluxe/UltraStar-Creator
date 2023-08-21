# UltraStar-Creator Development README

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/w64gbadl1kn6s7dt/branch/master?svg=true)](https://ci.appveyor.com/project/bohning/ultrastar-creator/branch/master)
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
#### Compiling on Windows
(under construction)

1. Install Qt6.
2. Build UltraStar-Creator
   * from command line: `cd src && qmake UltraStar-Creator.pro && make` 
   * using Qt Creator: open `src\UltraStar-Creator.pro` in Qt Creator, disable shadow build in Project tab, then build (CTRL+B)
3. Run `UltraStar-Creator.exe` in `bin\releease` and start creating new songs!

#### Compiling on Linux
(under construction)

1. Install Qt6.
2. Install dependencies: `sudo apt-get install libtag1-dev`
3. `git clone https://github.com/UltraStar-Deluxe/UltraStar-Creator.git`
4. `cd UltraStar-Creator`
5. Build UltraStar-Creator
   * from command line: `cd src && qmake6 UltraStar-Creator.pro && make` 
   * using Qt Creator: open `src/UltraStar-Creator.pro` in Qt Creator, disable shadow build in Project tab, then build (CTRL+B)
6. Run `UltraStar-Creator` in `bin/release` and start creating new songs!

#### Compiling on Mac OS X
(under construction)

1. Install homebrew via `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
2. Install the Qt framework via `brew install qt`.
3. Install external dependencies via `brew install taglib`
4. Build UltraStar-Creator
   * from command line: `cd src && qmake UltraStar-Creator.pro && make` 
   * using Qt Creator: open `src/UltraStar-Creator.pro` in Qt Creator, disable shadow build in Project tab, then build (CTRL+B)
5. Run `UltraStar-Creator.app` in `bin/release` and start creating new songs!

### 6. Contribute
Feel free to fork this project, modify it to your hearts content and maybe also do pull requests to this repository for additional features, improvements or clean-ups.
