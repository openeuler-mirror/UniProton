# Fetch a library into ../component/<dir> all or nothing: download and unpack
# into <dir>.tmp, run the optional prepare command there (generated headers,
# bare-metal patch), then move the finished tree into place. On any failure
# the temporary tree is removed and the script stops.
fetch_src()
{
    fetch_dir=${3:?}
    fetch_tmp=../component/${fetch_dir}.tmp
    rm -rf "$fetch_tmp" "../component/$fetch_dir"
    if mkdir "$fetch_tmp" && wget -q "$1" -O "$fetch_tmp/$2" && tar -zxf "$fetch_tmp/$2" -C "$fetch_tmp" &&
        (cd "$fetch_tmp" && ${4:-true}) && mv "$fetch_tmp/$fetch_dir" "../component/$fetch_dir"; then
        rm -rf "$fetch_tmp"
    else
        rm -rf "$fetch_tmp"
        echo "get $fetch_dir failed"
        exit 1
    fi
}

DEFCONFIG=../../../build/uniproton_config/config_armv8_sd3403/defconfig
# algo_lib <NAME>: is CONFIG_OS_SUPPORT_<NAME> enabled in the defconfig
algo_lib()
{
    grep -q "^CONFIG_OS_SUPPORT_$1=y" "$DEFCONFIG"
}

pushd ../../../src/net/
echo "################# get soem #################"
if [ -d "./soem" ] && [ -f "./soem/CMakeLists.txt" ]; then
    echo "soem already exists, skip download and patch."
else
    rm -rf ./soem
    git clone --depth=1 --filter=blob:none --sparse -b revert-2058de2 https://atomgit.com/openeuler/oee_archive.git
    cd oee_archive
    git sparse-checkout init --cone
    git sparse-checkout set "soem"
    cp soem/SOEM-1.4.0.tar.gz ../
    cd ../
    rm -rf oee_archive
    tar -zxf SOEM-1.4.0.tar.gz
    rm SOEM-1.4.0.tar.gz
    mv SOEM-1.4.0 soem
    cp UniProton-patch-for-soem.patch ./soem
    cd ./soem
    patch -p1 -d . < UniProton-patch-for-soem.patch
    rm -rf ../../../testsuites/soemTest/test
    cp -r test ../../../testsuites/soemTest/
fi
popd

if algo_lib EIGEN && [ ! -d "../component/eigen-3.4.0" ]
then
    echo "################# get eigen #################"
    fetch_src https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz eigen-3.4.0.tar.gz eigen-3.4.0
fi

if algo_lib EIQUADPROG && [ ! -d "../component/eiquadprog-1.3.2" ]
then
    echo "################# get eiquadprog #################"
    fetch_src https://github.com/stack-of-tasks/eiquadprog/archive/refs/tags/v1.3.2.tar.gz eiquadprog.tar.gz eiquadprog-1.3.2
fi
if algo_lib EIQUADPROG
then
    # generated headers: normally from upstream CMake; idempotent
    pushd ../component
    cat > eiquadprog-1.3.2/include/eiquadprog/config.hpp <<'CFG'
#ifndef EIQ_CONFIG_HPP
#define EIQ_CONFIG_HPP
#define EIQUADPROG_DLLAPI
#endif
CFG
    cat > eiquadprog-1.3.2/include/eiquadprog/deprecated.hpp <<'CFGD'
#ifndef EIQ_DEPRECATED_HPP
#define EIQ_DEPRECATED_HPP
#if defined(__GNUC__) || defined(__clang__)
#define EIQUADPROG_DEPRECATED __attribute__((deprecated))
#else
#define EIQUADPROG_DEPRECATED
#endif
#endif
CFGD
    popd
fi
