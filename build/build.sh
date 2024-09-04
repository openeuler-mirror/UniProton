#!/bin/bash
set -e
CUR_PATH=`pwd`
ARCH_TYPE=${1}
CPU_TYPE=${2}
# cortex ./build.sh cortex m4 sim helloworld
if [[ $3 == "sim" || $3 == "SIM" ]]; then
    SIM="_SIM_"
    APP=${4}
elif [ "$3" != "" ]; then
    SIM=""
    APP=${3}
else
    SIM=""
    APP=${CPU_TYPE}
fi

export HOME_PATH=$(dirname "$PWD")
export APP_VAR=${APP}
export SIM_VAR=${SIM}

function clean()
{
    if [ -d "${HOME_PATH}/output" ]; then
        rm -rf ${HOME_PATH}/output
    fi

    if [ -d "${HOME_PATH}/build/logs" ]; then
        rm -rf ${HOME_PATH}/build/logs
    fi

    if [ -d "${HOME_PATH}/build/output" ]; then
        rm -rf ${HOME_PATH}/build/output
    fi

    if [ -d "${HOME_PATH}/build/lua" ]; then
        rm -rf ${HOME_PATH}/build/lua
    fi

    if [ -d "${HOME_PATH}/build/modbus" ]; then
        rm -rf ${HOME_PATH}/build/modbus
    fi

    if [ -d "${HOME_PATH}/build/libcxx" ]; then
        rm -rf ${HOME_PATH}/build/libcxx
    fi

    if [ -d "${HOME_PATH}/build/build_forte" ]; then
        rm -rf ${HOME_PATH}/build/build_forte
    fi

    if [ -d "${HOME_PATH}/build/tools/__pycache__" ]; then
        rm -rf ${HOME_PATH}/build/tools/__pycache__
    fi

    if [ -d "${HOME_PATH}/build/libmetal" ]; then
        rm -rf ${HOME_PATH}/build/libmetal
    fi

    if [ -d "${HOME_PATH}/build/open-amp" ]; then
        rm -rf ${HOME_PATH}/build/open-amp
    fi

    find "${HOME_PATH}/build/" \( -name "*.asm" -o -name "*.bin" -o -name "*.elf" \) -type f -print0 | xargs -0 rm -f
}

# x86平台如果在oebuild上编译，需要打补丁
function patch_toolchain()
{
    # 获取交叉编译工具链,统一从config.xml获取,x86需要打补丁
    if [ "${ARCH_TYPE}" = "x86_64" ]; then
        gcc_file=/opt/buildtools/openeuler_gcc_x86_64/bin/x86_64-openeuler-linux-gnu-gcc
        if test -f "$gcc_file"; then
            echo "=========================x86_64 no need patch======================="
        else
            cp ${HOME_PATH}/src/component/patch/UniProton-patch-for-oebuild.patch ${HOME_PATH}/build/tools
            pushd ${HOME_PATH}/build/tools
            patch < UniProton-patch-for-oebuild.patch
            popd
        fi
    fi
}

# x86平台在oebuild平台上编译需要打patch
function get_toolchain()
{
    if [[ $(find "${HOME_PATH}/build/tools" -maxdepth 1 -name "*.patch" | wc -l) -eq 0 ]]; then
        patch_toolchain
    fi

    XPATH_EXPR="string(//project[@cpu_type=\"$CPU_TYPE\"]/platform/$CONFIG_COMPILE_PATH/text())"
    TOOLCHAIN_PATH=$(xmllint --xpath  "$XPATH_EXPR" ./tools/config.xml)/..
}

function musl_process()
{
    echo ' Build generic_include_file!'
    pushd ${HOME_PATH}/src/libc/musl
    mkdir -p include/bits/
    cp arch/generic/bits/*.h include/bits/
    cp arch/${CMAKE_SYSTEM_PROCESSOR}/bits/*.h include/bits/
    sed -f tools/mkalltypes.sed arch/${CMAKE_SYSTEM_PROCESSOR}/bits/alltypes.h.in include/alltypes.h.in > include/bits/alltypes.h
    cp arch/${CMAKE_SYSTEM_PROCESSOR}/bits/syscall.h.in include/bits/syscall.h
    sed -n -e s/__NR_/SYS_/p < arch/${CMAKE_SYSTEM_PROCESSOR}/bits/syscall.h.in >> include/bits/syscall.h
    popd
    echo ' End generic_include_file!'
}

function build_image()
{
    echo "================================================"
    echo "Will build ${ARCH_TYPE}_${CPU_TYPE}"
    echo "================================================"

    if [ -d "${HOME_PATH}/build/${CPU_TYPE}" ]; then
        rm -rf ${HOME_PATH}/build/${CPU_TYPE}
    fi

    # 获取交叉编译工具链,统一从config.xml获取
    get_toolchain
    echo "cross toolchain  ${TOOLCHAIN_PATH}"

    XPATH_EXPR="string(//project[@cpu_type=\"$CPU_TYPE\"]/platform/kconf_dir/text())"
    KCONF_DIR=$(xmllint --xpath  "$XPATH_EXPR" ./tools/config.xml)
    DEFCONFIG=${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/defconfig
    echo "DEFCONFIG PATH  ${DEFCONFIG}"

    # 组件获取
    sh ./auxiliary/build_fetch.sh ${DEFCONFIG} ${ARCH_TYPE}

    # 组件依赖处理
    pushd ${HOME_PATH}/build/tools
        python3 -c "import make_buildef; make_buildef.make_buildef('$HOME_PATH','$KCONF_DIR','CREATE')"
    popd

    musl_process

    # 组件编译
    sh ./auxiliary/build_component.sh $TOOLCHAIN_PATH ${ARCH_TYPE} ${CPU_TYPE} ${KCONF_DIR} ${DEFCONFIG}

    # 内核编译, 删除上次生成的内核静态库以及头文件, 然后重新编
    rm -rf ${HOME_PATH}/build/output
    pushd ${HOME_PATH}/build/tools
        python3 build.py $CPU_TYPE
    popd

    # 编译后处理
    cp ${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/$APP $APP.elf

    if [[ "${APP}" == "cxxTest" || "${APP}" == "eigenTest" ]]; then
        python ./tools/bin_helper.py -f ./$APP.elf --nocopy
    fi

    if [[ ${ARCH_TYPE} == "armv8" ]]; then
        $TOOLCHAIN_PATH/bin/aarch64-openeuler-linux-objcopy -O binary ./$APP.elf $APP.bin
        $TOOLCHAIN_PATH/bin/aarch64-openeuler-linux-objdump -D ./$APP.elf > $APP.asm
    elif [ "${ARCH_TYPE}" == "cortex" ]; then
        $TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary ./$APP.elf $APP.bin
    elif [ "${ARCH_TYPE}" == "x86_64" ]; then
        $TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-objcopy -O binary ./$APP.elf $APP.bin
        $TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-objdump -S ./$APP.elf > $APP.asm

        # 编译完成，x86 oebuild平台补丁恢复
        if [[ $(find "${HOME_PATH}/build/tools" -maxdepth 1 -name "*.patch" | wc -l) -gt 0 ]]; then
            pushd ${HOME_PATH}/build/tools
                patch -R < UniProton-patch-for-oebuild.patch
                rm -f UniProton-patch-for-oebuild.patch
            popd
        fi
    elif [ "${ARCH_TYPE}" == "riscv64" ]; then
        $TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objcopy -O binary ./$APP.elf $APP.bin
        $TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objdump -D ./$APP.elf > $APP.asm
    else
        echo "=========================no objcopy======================="
    fi

    echo "================================================"
    echo "Build ${ARCH_TYPE}_${CPU_TYPE} successfully"
    echo "================================================"
}

function usage()
{
    echo "Usage:"
    echo "    ./build.sh arch_type cpu_type [app_name]"
    echo "               arch_type:armv8 x86_64 cortex riscv64"
    echo "               cpu_type:ascend310b"
    echo "               app_name:ascend310b UniPorton_test_proxy_posix_interface"
    echo "eg. ./build.sh armv8 ascend310b"
    echo "    ./build.sh armv8 ascend310b UniPorton_test_proxy_posix_interface"
    echo "    ./build.sh x86_64 uvp"
    echo "    ./build.sh cortex m4 fatfs"
    echo "    ./build.sh cortex m4 sim helloworld"
    echo "    ./build.sh clean"
}


case ${ARCH_TYPE} in
    "armv8")
        CONFIG_COMPILE_PATH="compile_path_arm64"
        CMAKE_SYSTEM_PROCESSOR=aarch64
        build_image
        ;;
    "x86_64")
        CONFIG_COMPILE_PATH="compile_path_x86"
        CMAKE_SYSTEM_PROCESSOR=x86_64
        build_image
        ;;
    "cortex")
        CONFIG_COMPILE_PATH="compile_path_arm64"
        CMAKE_SYSTEM_PROCESSOR=arm
        build_image
        ;;
    "riscv64")
        CONFIG_COMPILE_PATH="compile_path_riscv64"
        CMAKE_SYSTEM_PROCESSOR=riscv64
        build_image
        ;;
    "clean")
        clean
        ;;
    "help")
        usage
        ;;
    *)
        # 默认情况，如果上面的情况都不匹配
        usage
        ;;
esac

