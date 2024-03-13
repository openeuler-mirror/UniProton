# Allowed compilation APP:
# x86_64 UniPorton_test_posix_time_interface UniPorton_test_proxy_posix_interface UniPorton_test_libxml2_interface
# task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency
# linuxTest ethercatTest
export ALL="x86_64"

gcc_file=/opt/buildtools/openeuler_gcc_x86_64/bin/x86_64-openeuler-linux-gnu-gcc
if test -f "$gcc_file"; then
    export TOOLCHAIN_PATH=/opt/buildtools/openeuler_gcc_x86_64
else
    export TOOLCHAIN_PATH=/usr1/openeuler/gcc/openeuler_gcc_x86_64
    # oebuild environment
    cp x86_64-patch-for-oebuild.patch ./../../../
    pushd ./../../../
    patch -N -p1 -d . < x86_64-patch-for-oebuild.patch
    popd
fi

# if yocto build, do_fetch step will download code
if [ "$2" != "yocto" ]
then
    sh ./build_fetch.sh
fi
sh ./build_static.sh x86_64
sh ./build_openamp.sh $TOOLCHAIN_PATH

function build()
{
    export APP=$1
    export TMP_DIR=$APP

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="x86_64" -DCMAKE_C_COMPILER=$TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-gcc -DCMAKE_CXX_COMPILER=$TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-g++
    pushd $TMP_DIR
    make $APP
    popd

    cp ./$TMP_DIR/$APP $APP.elf

    if [ "${APP}" == "cxxTest" ]
    then
        python ./../../../bin_helper.py -f ./$APP.elf --nocopy
    fi

    $TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-objcopy -O binary ./$APP.elf $APP.bin
    $TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-objdump -S ./$APP.elf > $APP.asm

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
