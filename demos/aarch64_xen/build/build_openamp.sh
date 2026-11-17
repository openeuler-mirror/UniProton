#!/usr/bin/env bash
set -euo pipefail
HERE=$(cd -- "$(dirname -- "$0")" && pwd)
DEMO=$(cd "$HERE/.." && pwd)
UNI=$(cd "$DEMO/../.." && pwd)
export TOOLCHAIN_PATH=${TOOLCHAIN_PATH:-/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf}
TC="$HERE/uniproton_arm64_gcc.cmake"
JOBS=${JOBS:-8}
export UNIPROTON_INCLUDE_FLAGS="-I\"$DEMO/include\" -I\"$UNI/output/libc/include\" -I\"$UNI/platform/libboundscheck/include\" -I\"$DEMO/config\""

# The pinned official packaging revisions still carry these exact releases.
# Only the existing UniProton patches are applied to demo-local source copies.
stage_dependency() {
    local name=$1 input=$2 archive=$3 url=$4 ref=$5 release=$6 patch_name=$7
    local target="$DEMO/component/$name" package="$HERE/$name/package"
    local source="$target"
    if [[ ! -d $source ]]; then
        source=$(mktemp -d "$DEMO/component/$name-stage.XXXXXX")
        if [[ -n $input ]]; then
            cp -a "$input/." "$source/"
        else
            if [[ -z $archive ]]; then
                if [[ ! -d $package ]]; then git clone "$url" "$package"; fi
                git -C "$package" checkout --detach "$ref"
                archive="$package/$release.tar.gz"
            fi
            # Missing historical archives are errors, never silently upgraded.
            test -f "$archive"
            tar -xf "$archive" --strip-components=1 -C "$source" "$release/"
        fi
    fi
    local patch_file="$DEMO/component/UniProton-patch-for-$patch_name.patch"
    if patch --dry-run --batch --forward -p1 -d "$source" < "$patch_file" >/dev/null; then
        patch --batch --forward -p1 -d "$source" < "$patch_file"
    else
        # An incremental invocation must already contain the exact patch.
        patch --dry-run --batch --reverse -p1 -d "$source" < "$patch_file" >/dev/null
    fi
    if [[ $source != "$target" ]]; then mv -- "$source" "$target"; fi
}
stage_dependency libmetal "${LIBMETAL_SOURCE_DIR:-}" "${LIBMETAL_ARCHIVE:-}" \
    https://gitee.com/src-openeuler/libmetal.git \
    70980a0045d3a4be1260410544db95fbf070071f libmetal-2022.10.0 libmetal
stage_dependency open-amp "${OPENAMP_SOURCE_DIR:-}" "${OPENAMP_ARCHIVE:-}" \
    https://gitee.com/src-openeuler/OpenAMP.git \
    de0c82a1c443989893383b8dead0eaa611e61cd2 openamp-2022.10.1 openamp

cmake -S "$DEMO/component/libmetal" -B "$HERE/libmetal/build" \
    -DCMAKE_TOOLCHAIN_FILE="$TC" -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DPROJECT_SYSTEM=uniproton -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF \
    -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
cmake --build "$HERE/libmetal/build" -j "$JOBS"
DESTDIR="$HERE/libmetal/output" cmake --install "$HERE/libmetal/build"
cmake -S "$DEMO/component/open-amp" -B "$HERE/open-amp/build" \
    -DCMAKE_TOOLCHAIN_FILE="$TC" -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DWITH_DOC=OFF -DWITH_APPS=OFF -DWITH_PROXY=OFF -DWITH_SHARED_LIB=OFF \
    -DCMAKE_PREFIX_PATH="$HERE/libmetal/output/usr/local" \
    -DLIBMETAL_INCLUDE_DIR="$HERE/libmetal/output/usr/local/include" \
    -DLIBMETAL_LIB="$HERE/libmetal/output/usr/local/lib/libmetal.a" \
    -DLIBMETAL_LIB_DIR="$HERE/libmetal/output/usr/local/lib"
cmake --build "$HERE/open-amp/build" -j "$JOBS"
DESTDIR="$HERE/open-amp/output" cmake --install "$HERE/open-amp/build"

# Standalone libmica is the only extra dependency step for this demo.
if grep -q '^CONFIG_OS_OPTION_LIBMICA=y$' "$UNI/build/uniproton_config/config_armv8_aarch64_xen/defconfig"; then
    if [[ -z ${MCS_SOURCE_DIR:-} ]]; then
        MCS_SOURCE_DIR="$DEMO/component/mcs"
        MCS_URL=${MCS_URL:-https://atomgit.com/openeuler/mcs.git}
        MCS_REF=${MCS_REF:-c1707dd7ef8879915965d6bba91aed9f1e7df305}
        if [[ ! -d $MCS_SOURCE_DIR ]]; then git clone "$MCS_URL" "$MCS_SOURCE_DIR"; fi
        git -C "$MCS_SOURCE_DIR" checkout --detach "$MCS_REF"
    fi
    cmake -S "$MCS_SOURCE_DIR/rtos/libmica" -B "$HERE/mica" \
        -DCMAKE_TOOLCHAIN_FILE="$TC" -DCMAKE_INSTALL_PREFIX="$HERE/mica-install" \
        -DPROJECT_SYSTEM=uniproton -DMICA_PED=xen -DMICA_XEN_PROVIDER=builtin \
        -DPLATFORM_INCLUDE_DIRS="$UNI/src/component/libmica_adapter/include;$DEMO/include;$UNI/output/libc/include;$UNI/platform/libboundscheck/include;$DEMO/config" \
        -DPLATFORM_DEP_INCLUDE_DIRS="$HERE/libmetal/output/usr/local/include;$HERE/open-amp/output/usr/local/include" \
        -DLIBMETAL_PATH="$HERE/libmetal" -DOPENAMP_PATH="$HERE/open-amp"
    cmake --build "$HERE/mica" -j "$JOBS"
    cmake --install "$HERE/mica"
fi
