#!/usr/bin/env bash
set -euo pipefail
HERE=$(cd -- "$(dirname -- "$0")" && pwd)
DEMO=$(cd "$HERE/.." && pwd)
UNI=$(cd "$DEMO/../.." && pwd)
CPU=${1:-aarch64_xen}
[[ $CPU == aarch64_xen ]] || { echo "Unsupported CPU: $CPU" >&2; exit 1; }
if [[ -z ${BOUNDSCHECK_SOURCE_DIR:-} ]]; then
    mkdir -p "$HERE/libmetal"
    BOUNDSCHECK_SOURCE_DIR="$HERE/libmetal/libboundscheck"
    if [[ ! -d $BOUNDSCHECK_SOURCE_DIR ]]; then
        git clone https://atomgit.com/openeuler/libboundscheck.git "$BOUNDSCHECK_SOURCE_DIR"
    fi
fi
mkdir -p "$UNI/platform/libboundscheck/include" "$UNI/platform/libboundscheck/src" "$DEMO/include" "$DEMO/libs"
cp -a "$BOUNDSCHECK_SOURCE_DIR/include/." "$UNI/platform/libboundscheck/include/"
cp -a "$BOUNDSCHECK_SOURCE_DIR/include/." "$DEMO/include/"
cp -a "$BOUNDSCHECK_SOURCE_DIR/src/." "$UNI/platform/libboundscheck/src/"
cd "$UNI"
python build.py "$CPU"
cp output/UniProton/lib/$CPU/* "$DEMO/libs/"
cp output/libboundscheck/lib/$CPU/* "$DEMO/libs/"
cp -a src/include/uapi/. "$DEMO/include/"
cp build/uniproton_config/config_armv8_aarch64_xen/prt_buildef.h "$DEMO/include/"
