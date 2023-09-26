export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf
# export ALL="task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency"
export ALL="rk3568_jailhouse task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency"

sh ./build_static.sh rk3568_jailhouse

function build()
{
    export APP=$1
    export TMP_DIR=$APP

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="rk3568_jailhouse"
    pushd $TMP_DIR
    make $APP
    popd

    cp ./$TMP_DIR/$APP $APP.elf

    $TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy -O binary ./$APP.elf $APP.bin
    $TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump -D ./$APP.elf > $APP.asm
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
