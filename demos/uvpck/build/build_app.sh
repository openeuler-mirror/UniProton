export TOOLCHAIN_PATH=/usr1/openeuler/gcc/openeuler_gcc_x86_64
export ALL="uvpck"

sh ./build_static.sh uvpck
sh ./build_openamp.sh $TOOLCHAIN_PATH

function build()
{
    export APP=$1
    export TMP_DIR=$APP

    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH
    pushd $TMP_DIR
    make $APP
    popd
    cp ./$TMP_DIR/$APP $APP.elf
    $TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-objcopy -O binary ./$APP.elf $APP.bin
    $TOOLCHAIN_PATH/bin/x86_64-openeuler-linux-gnu-objdump -D ./$APP.elf > $APP.asm
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
