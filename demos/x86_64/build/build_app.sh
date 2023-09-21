export TOOLCHAIN_PATH=/usr1/openeuler/gcc/openeuler_gcc_x86_64

# Allowed compilation APP:
# x86_64 UniPorton_test_posix_time_interface UniPorton_test_proxy_posix_interface
# task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency
# linuxTest ethercatTest
export ALL="linuxTest ethercatTest x86_64"

sh ./build_static.sh x86_64
sh ./build_openamp.sh $TOOLCHAIN_PATH

function build()
{
    export APP=$1
    export TMP_DIR=$APP

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="x86_64"
    pushd $TMP_DIR
    make $APP
    popd
    if [ "$APP" == "task-switch" ] || [ "$APP" == "task-preempt" ] || [ "$APP" == "semaphore-shuffle" ] ||
        [ "$APP" == "interrupt-latency" ] || [ "$APP" == "deadlock-break" ] || [ "$APP" == "message-latency" ]
    then
        cp ./$TMP_DIR/testsuites/$APP $APP.elf
    else
        cp ./$TMP_DIR/$APP $APP.elf
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
