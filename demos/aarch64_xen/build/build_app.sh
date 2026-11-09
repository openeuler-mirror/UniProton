#!/usr/bin/env bash
set -euo pipefail
HERE=$(cd -- "$(dirname -- "$0")" && pwd)
DEMO=$(cd "$HERE/.." && pwd)
UNI=$(cd "$DEMO/../.." && pwd)
export TOOLCHAIN_PATH=${TOOLCHAIN_PATH:-/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf}
export PATH="$TOOLCHAIN_PATH/bin:$PATH"
ALL="aarch64_xen"
cd "$HERE"
bash ./build_static.sh aarch64_xen
bash ./build_openamp.sh

build() {
    local APP=$1
    if [[ $APP != aarch64_xen ]]; then
        printf 'Unknown app: %s\n' "$APP" >&2
        return 1
    fi
    local TMP_DIR="$HERE/$APP"
    cmake -S "$DEMO" -B "$TMP_DIR" -DAPP="$APP" \
        -DTOOLCHAIN_PATH="$TOOLCHAIN_PATH" -DCPU_TYPE=aarch64_xen \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
        -DMICA_INSTALL_PREFIX="$HERE/mica-install" \
        -DMICA_OPENAMP_ROOT="$HERE/open-amp/output/usr/local" \
        -DMICA_LIBMETAL_ROOT="$HERE/libmetal/output/usr/local" \
        -DMICA_UNIPROTON_ARCHIVE="$DEMO/libs/libAARCH64_XEN.a" \
        -DMICA_BOUNDSCHECK_ARCHIVE="$DEMO/libs/libCortexMXsec_c.lib" \
        -DUNIPROTON_INCLUDE_DIRS="$DEMO/include;$UNI/output/libc/include;$UNI/platform/libboundscheck/include;$DEMO/config"
    cmake --build "$TMP_DIR" --target "$APP" -j "${JOBS:-8}"
    cp "$TMP_DIR/$APP" "$HERE/$APP.elf"
    cp "$TMP_DIR/$APP.map" "$HERE/$APP.map"
    "$TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy" -O binary "$HERE/$APP.elf" "$HERE/$APP.bin"
    "$TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump" -D "$HERE/$APP.elf" > "$HERE/$APP.asm"
    # APP is validated above; only this app's successful temporary build is removed.
    rm -rf -- "$TMP_DIR"
}

if [[ ${1:-} == all || -z ${1:-} ]]; then
    for app in $ALL; do build "$app"; done
else
    build "$1"
fi
