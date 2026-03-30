#!/usr/bin/env bash
set -euo pipefail

log() { echo "==> $1"; }

download() {
    local url="$1"
    local out="$2"

    if [[ -f "$out" ]]; then
        log "Already downloaded: $out"
    else
        curl -L -C - -o "$out" "$url"
    fi
}

extract_zip() {
    local zip="$1"
    local dest="$2"

    if [[ -d "$dest" ]]; then
        log "Already extracted: $dest"
    else
        unzip -q "$zip" -d "$dest"
    fi
}

echo "==> Detecting platform..."
OS="$(uname -s)"

case "$OS" in
    MINGW*|MSYS*|CYGWIN*)
        PLATFORM="windows"
        ;;
    Darwin)
        PLATFORM="macos"
        ;;
    Linux)
        PLATFORM="linux"
        ;;
    *)
        echo "Unsupported OS: $OS"
        exit 1
        ;;
esac

echo "==> Platform: $PLATFORM"

TEMP_DIR="./.tmp"
mkdir -p "$TEMP_DIR"
LIB_DIR="./lib/$PLATFORM"
mkdir -p "$LIB_DIR"

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

if [[ "$PLATFORM" == "windows" ]]; then
    echo "==> Installing dependencies via Chocolatey..."
    choco install -y nsis

elif [[ "$PLATFORM" == "macos" ]]; then
    echo "==> Installing Homebrew dependencies..."
    brew update
    brew bundle --file=- --no-upgrade <<EOF
brew "taglib"
brew "create-dmg"
brew "qt"
EOF

elif [[ "$PLATFORM" == "linux" ]]; then
    echo "==> Installing apt dependencies..."
    sudo apt update -qq
    sudo apt install -y \
        libgl1-mesa-dev \
        build-essential \
        libtag1-dev \
        libcld2-dev \
        libxcb-icccm4 \
        libxcb-image0 \
        libxcb-keysyms1 \
        libxcb-render-util0 \
        libxcb-xinerama0 \
        libxcb-xkb-dev \
        libxkbcommon-x11-0 \
        libxcb-cursor0 \
        libgtk2.0-dev \
        libfuse2
fi

# --------------------------------------------------
# BASS
# --------------------------------------------------

log "Setting up BASS..."
LIB_NAME="bass"

if [[ "$PLATFORM" == "windows" ]]; then
    BASS_URL="https://www.un4seen.com/files/bass24.zip"
    LIB_FILE="$LIB_NAME.dll"

elif [[ "$PLATFORM" == "macos" ]]; then
    BASS_URL="https://www.un4seen.com/files/bass24-osx.zip"
    LIB_FILE="lib$LIB_NAME.dylib"

elif [[ "$PLATFORM" == "linux" ]]; then
    BASS_URL="https://www.un4seen.com/files/bass24-linux.zip"
    LIB_FILE="lib$LIB_NAME.so"
fi

download "$BASS_URL" "$TEMP_DIR/$LIB_NAME.zip"
extract_zip "$TEMP_DIR/$LIB_NAME.zip" "$TEMP_DIR/$LIB_NAME-tmp"

mkdir -p "$INCLUDE_DIR/$LIB_NAME"
cp -n "$TEMP_DIR/$LIB_NAME-tmp/c/$LIB_NAME.h" "$INCLUDE_DIR/$LIB_NAME/" || true

if [[ "$PLATFORM" == "windows" ]]; then
    cp -n "$TEMP_DIR/$LIB_NAME-tmp/x64/$LIB_NAME.dll" "$LIB_DIR/" || true
    cp -n "$TEMP_DIR/$LIB_NAME-tmp/x64/$LIB_NAME.lib" "$LIB_DIR/" || true
else
    cp -n "$TEMP_DIR/$LIB_NAME-tmp/$LIB_FILE" "$LIB_DIR/" || true
fi

# --------------------------------------------------
# BASS_FX
# --------------------------------------------------

log "Setting up BASS_FX..."
LIB_NAME="bass_fx"

if [[ "$PLATFORM" == "windows" ]]; then
    BASS_FX_URL="https://www.un4seen.com/files/z/0/bass_fx24.zip"
    LIB_FILE="$LIB_NAME.dll"

elif [[ "$PLATFORM" == "macos" ]]; then
    BASS_FX_URL="https://www.un4seen.com/files/z/0/bass_fx24-osx.zip"
    LIB_FILE="lib$LIB_NAME.dylib"

elif [[ "$PLATFORM" == "linux" ]]; then
    BASS_FX_URL="https://www.un4seen.com/files/z/0/bass_fx24-linux.zip"
    LIB_FILE="lib$LIB_NAME.so"
fi

download "$BASS_FX_URL" "$TEMP_DIR/$LIB_NAME.zip"
extract_zip "$TEMP_DIR/$LIB_NAME.zip" "$TEMP_DIR/$LIB_NAME-tmp"

mkdir -p "$INCLUDE_DIR/$LIB_NAME"
cp -n "$TEMP_DIR/$LIB_NAME-tmp/bass_fx.h" "$INCLUDE_DIR/$LIB_NAME/" || true

if [[ "$PLATFORM" == "windows" ]]; then
    cp -n "$TEMP_DIR/$LIB_NAME-tmp/x64/$LIB_NAME.dll" "$LIB_DIR/" || true
    cp -n "$TEMP_DIR/$LIB_NAME-tmp/x64/$LIB_NAME.lib" "$LIB_DIR/" || true
else
    cp -n "$TEMP_DIR/$LIB_NAME-tmp/$LIB_FILE" "$LIB_DIR/" || true
fi

# --------------------------------------------------
# cld2 - Compact Language Detector 2
# --------------------------------------------------

log "Setting up cld2..."
LIB_NAME="cld2"

CLD2_PATH="include/$LIB_NAME"
CLD2_URL="https://github.com/CLD2Owners/cld2.git"

if [ ! -d "$CLD2_PATH/.git" ]; then
    echo "Cloning cld2..."
    git clone --depth 1 $CLD2_URL $CLD2_PATH
else
    echo "cld2 already exists, updating..."
    cd $CLD2_PATH && git pull && cd -
fi

if [[ "$PLATFORM" == "windows" ]]; then
    if [[ -f "$LIB_DIR/$PLATFORM/$LIB_NAME.lib" && -f "$LIB_DIR/$PLATFORM/$LIB_NAME.dll" ]]; then
        log "$LIB_NAME already built (Windows)"
    else
        log "Compiling $LIB_NAME (Windows)..."

        pushd "$INCLUDE_DIR/$LIB_NAME/internal/" > /dev/null

        # Compile only if no object files exist
        if ! ls *.obj >/dev/null 2>&1; then
            cl /TP /GR /EHsc /c /MD \
              cldutil.cc cldutil_shared.cc compact_lang_det.cc \
              compact_lang_det_hint_code.cc compact_lang_det_impl.cc \
              debug.cc fixunicodevalue.cc generated_entities.cc \
              generated_language.cc generated_ulscript.cc \
              getonescriptspan.cc lang_script.cc offsetmap.cc \
              scoreonescriptspan.cc tote.cc utf8statetable.cc \
              cld_generated_cjk_uni_prop_80.cc \
              cld2_generated_cjk_compatible.cc \
              cld_generated_cjk_delta_bi_32.cc \
              generated_distinct_bi_0.cc \
              cld2_generated_quad0122.cc \
              cld2_generated_deltaocta0122.cc \
              cld2_generated_distinctocta0122.cc \
              cld_generated_score_quad_octa_0122.cc
        else
            log "Object files already exist, skipping compilation"
        fi

        # Link (always safe to rerun)
        link /dll /out:$LIB_NAME.dll *.obj
        lib /out:$LIB_NAME.lib *.obj

        popd > /dev/null

        cp "$INCLUDE_DIR/$LIB_NAME/internal/$LIB_NAME.lib" "$LIB_DIR/"
        cp "$INCLUDE_DIR/$LIB_NAME/internal/$LIB_NAME.dll" "$LIB_DIR/"
    fi

elif [[ "$PLATFORM" == "macos" ]]; then
    if [[ ! -f "$LIB_DIR/lib$LIB_NAME.dylib" ]]; then
        log "Compiling $LIB_NAME..."

        pushd "$INCLUDE_DIR/$LIB_NAME/internal/" > /dev/null
        log "Patching cld for macOS..."
        patch compile_libs.sh ../../$LIB_NAME-mac-compile.patch
        export CFLAGS="-Wno-narrowing -O3"
        ./compile_libs.sh

        install_name_tool -id @executable_path/lib$LIB_NAME.dylib lib$LIB_NAME.dylib 2>/dev/null || true

        popd > /dev/null

        cp "$INCLUDE_DIR/$LIB_NAME/internal/lib$LIB_NAME.dylib" "$LIB_DIR/"
    else
        log "$LIB_DIR/lib$LIB_NAME.dylib already present"
    fi

elif [[ "$PLATFORM" == "linux" ]]; then
    log "Using system $LIB_NAME (Linux)"
fi

# --------------------------------------------------
# SRT Parser
# --------------------------------------------------

log "Setting up SRT parser..."
LIB_NAME="srtparser"

if [[ ! -f "$INCLUDE_DIR/$LIB_NAME/$LIB_NAME.h" ]]; then
    mkdir -p "$INCLUDE_DIR/$LIB_NAME"
    curl -L -o "$INCLUDE_DIR/$LIB_NAME/$LIB_NAME.h" \
      https://raw.githubusercontent.com/saurabhshri/simple-yet-powerful-srt-subtitle-parser-cpp/master/srtparser.h
else
    log "$INCLUDE_DIR/$LIB_NAME/$LIB_NAME.h already present"
fi

# --------------------------------------------------
# taglib
# --------------------------------------------------

log "Setting up taglib..."
LIB_NAME="taglib"

if [[ "$PLATFORM" == "windows" ]]; then
    if [[ -f "$LIB_DIR/tag.lib" && -f "$LIB_DIR/tag.dll" ]]; then
        log "$LIB_NAME already built"
    else
        log "Building $LIB_NAME (Windows)..."

        # Clone only if missing
        if [[ ! -d "$TEMP_DIR/$LIB_NAME" ]]; then
            git clone --recurse-submodules https://github.com/taglib/taglib.git "$TEMP_DIR/$LIB_NAME"
        else
            log "$LIB_NAME repo already present"
        fi

        pushd "$TEMP_DIR/$LIB_NAME" > /dev/null

        # Configure only if build dir missing
        if [[ ! -d build ]]; then
            cmake -B build \
                -G "Visual Studio 17 2022" \
                -A x64 \
                -DWITH_ZLIB=OFF \
                -DBUILD_SHARED_LIBS=ON \
                -DENABLE_STATIC_RUNTIME=OFF \
                -DBUILD_TESTING=OFF
        else
            log "CMake already configured"
        fi

        # Build (safe to rerun)
        cmake --build build --config Release

        popd > /dev/null

        # --------------------------------------------------
        # Copy headers
        # --------------------------------------------------

        mkdir -p "$INCLUDE_DIR/$LIB_NAME"

        if [[ ! -f "$INCLUDE_DIR/$LIB_NAME/$LIB_NAME.h" ]]; then
            cp -r "$TEMP_DIR/$LIB_NAME/$LIB_NAME/"* "$INCLUDE_DIR/$LIB_NAME/"
        else
            log "$LIB_NAME headers already present"
        fi

        # --------------------------------------------------
        # Copy binaries
        # --------------------------------------------------

        mkdir -p "$LIB_DIR/"

        cp -f "$TEMP_DIR/$LIB_NAME/build/$LIB_NAME/Release/tag.dll" "$LIB_DIR/"
        cp -f "$TEMP_DIR/$LIB_NAME/build/$LIB_NAME/Release/tag.lib" "$LIB_DIR/"
    fi
elif [[ "$PLATFORM" == "macos" ]]; then
    log "Using brew installation of taglib"
elif [[ "$PLATFORM" == "linux" ]]; then
    log "Using system installation of taglib"
fi

# --------------------------------------------------
# Qt rpath fix (macOS only)
# --------------------------------------------------

if [[ "$PLATFORM" == "macos" ]]; then
    echo "==> Fixing Qt lib rpaths..."

    QT_PREFIX="$(brew --prefix)"

    for framework in QtCore QtGui QtNetwork QtWidgets QtPdf QtSvg QtVirtualKeyboard QtQuick QtQmlModels QtQml QtOpenGL QtMultimedia; do
        lib="$QT_PREFIX/lib/$framework.framework/Versions/A/$framework"
        if [[ -f "$lib" ]]; then
            install_name_tool -id "@rpath/$framework.framework/Versions/A/$framework" "$lib" 2>/dev/null || true
        fi
    done
fi

# --------------------------------------------------
# Done
# --------------------------------------------------

echo ""
echo "==> Setup complete."

if [[ "$PLATFORM" == "macos" ]]; then
    echo "Build with:"
    echo "  cd src && qmake6 UltraStar-Creator.pro && make -j\$(sysctl -n hw.logicalcpu)"
elif [[ "$PLATFORM" == "linux" ]]; then
    echo "Build with:"
    echo "  cd src && qmake6 UltraStar-Creator.pro && make -j\$(nproc)"
else
    echo "Build with (MSVC):"
    echo "  cd src && qmake6 UltraStar-Creator.pro -spec win32-msvc && nmake"
fi