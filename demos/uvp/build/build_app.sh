# Allowed compilation APP:
# x86_64 UniPorton_test_posix_time_interface UniPorton_test_proxy_posix_interface UniPorton_test_libxml2_interface
# task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency
# linuxTest ethercatTest
export ALL="x86_64"

cp x86_64-toolchain-patch.patch ./../../../
pushd ./../../../
patch -N -p1 -d . < x86_64-toolchain-patch.patch
rm x86_64-toolchain-patch.patch
popd

TOOLCHAIN_PATH=/usr1/openeuler/gcc/openeuler_gcc_x86_64/bin
CROSS_TOOLCHAIN_PREFIX=$TOOLCHAIN_PATH/x86_64-openeuler-linux-gnu

CXX=$CROSS_TOOLCHAIN_PREFIX-g++
CC=$CROSS_TOOLCHAIN_PREFIX-gcc
AS=$CROSS_TOOLCHAIN_PREFIX-as
OBJCOPY=$CROSS_TOOLCHAIN_PREFIX-objcopy
OBJDUMP=$CROSS_TOOLCHAIN_PREFIX-objdump

# if yocto build, do_fetch step will download code
if [ "$2" != "yocto" ]
then
    sh ./build_fetch.sh
fi

sh ./build_static.sh uvp
sh ./build_openamp.sh $CROSS_TOOLCHAIN_PREFIX
sh ./build_lwip.sh $TOOLCHAIN_PATH

function build()
{
    export APP=$1
    export TMP_DIR=$APP

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DCPU_TYPE:SRTING="x86_64" \
        -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH \
        -DCMAKE_CXX_COMPILER:STRING=$CXX -DCMAKE_C_COMPILER:STRING=$CC -DCMAKE_ASM_COMPILER:STRING=$AS
    pushd $TMP_DIR
    make $APP
    popd

    cp ./$TMP_DIR/$APP $APP.elf

    if [ "${APP}" == "cxxTest" ]
    then
        python ./../../../bin_helper.py -f ./$APP.elf --nocopy
    fi

    $OBJCOPY -O binary ./$APP.elf $APP.bin
    $OBJDUMP -Sldx ./$APP.elf > $APP.asm

    rm -rf $TMP_DIR
}

if [ "$1" == "all" ] || [ "$1" == "" ]
then
    for i in $ALL
    do
        build $i
    done
elif [ "$1" != "" ]
then
    build $1
fi
