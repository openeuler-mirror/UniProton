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

if algo_lib PINOCCHIO && [ ! -d "../component/boost_1_76_0" ]
then
    echo "################# get boost (headers only) #################"
    fetch_src https://archives.boost.io/release/1.76.0/source/boost_1_76_0.tar.gz boost_1_76_0.tar.gz boost_1_76_0
fi

if algo_lib PINOCCHIO; then
    if [ ! -d "../component/pinocchio-2.6.21" ]; then
        echo "################# get pinocchio #################"
        pinocchio_patch()
        {
            patch -d pinocchio-2.6.21 -p1 < ../UniProton-patch-for-pinocchio.patch
        }
        fetch_src https://github.com/stack-of-tasks/pinocchio/archive/refs/tags/v2.6.21.tar.gz pinocchio.tar.gz pinocchio-2.6.21 pinocchio_patch
    fi
    # generated headers, normally from upstream CMake: idempotent
    pushd ../component
    cat > pinocchio-2.6.21/include/pinocchio/config.hpp <<'CFG'
#ifndef PINOCCHIO_BAREMETAL_CONFIG_HPP
#define PINOCCHIO_BAREMETAL_CONFIG_HPP
#define PINOCCHIO_VERSION "2.6.21"
#define PINOCCHIO_VERSION_MAJOR 2
#define PINOCCHIO_VERSION_MINOR 6
#define PINOCCHIO_VERSION_PATCH 21
#define PINOCCHIO_BAREMETAL 1
#endif
CFG
    cat > pinocchio-2.6.21/include/pinocchio/deprecated.hpp <<'CFG2'
#ifndef PINOCCHIO_DEPRECATED_HPP
#define PINOCCHIO_DEPRECATED_HPP
#if defined(__GNUC__) || defined(__clang__)
#define PINOCCHIO_DEPRECATED __attribute__((deprecated))
#else
#define PINOCCHIO_DEPRECATED
#endif
#define PINOCCHIO_UNUSED __attribute__((unused))
#endif
CFG2
    cat > pinocchio-2.6.21/include/pinocchio/warning.hpp <<'CFG3'
#ifndef PINOCCHIO_WARNING_HPP
#define PINOCCHIO_WARNING_HPP
#define PINOCCHIO_PRAGMA(x) _Pragma(#x)
#define PINOCCHIO_STRINGIZE(x) #x
#endif
CFG3
    popd
fi

if algo_lib PROXSUITE && [ ! -d "../component/proxsuite-0.6.7" ]
then
    echo "################# get proxsuite #################"
    proxsuite_config()
    {
        cat > proxsuite-0.6.7/include/proxsuite/config.hpp <<'CFG'
#ifndef PROXSUITE_BAREMETAL_CONFIG_HPP
#define PROXSUITE_BAREMETAL_CONFIG_HPP
#define PROXSUITE_VERSION "0.6.7"
/* vectorization disabled: sd3403 builds with -march=armv8.2-a+nosimd */
#endif
CFG
    }
    fetch_src https://github.com/Simple-Robotics/proxsuite/archive/refs/tags/v0.6.7.tar.gz proxsuite.tar.gz proxsuite-0.6.7 proxsuite_config
fi
