export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf
export TOOLCHAIN_GCC_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
# export ALL="task-switch task-preempt semaphore-shuffle interrupt-latency deadlock-break message-latency"
export ALL="rk3568_jailhouse"

sh ./build_static.sh rk3568_jailhouse
sh ./build_openamp.sh $TOOLCHAIN_PATH

pushd ./../../../src/component/lua-5.3.4/src
    make posix
    make echo
    cp ./liblua.a ./../../../../demos/rk3568_jailhouse/libs/
    cp ./lua.h ./../../../../demos/rk3568_jailhouse/include/
    cp ./luaconf.h ./../../../../demos/rk3568_jailhouse/include/
    cp ./lualib.h ./../../../../demos/rk3568_jailhouse/include/
    cp ./lauxlib.h ./../../../../demos/rk3568_jailhouse/include/
    cp ./lua.hpp ./../../../../demos/rk3568_jailhouse/include/

    # 临时拷贝，用于排查lua依赖
    cp ./lprefix.h ./../../../../demos/rk3568_jailhouse/include/
    cp ./lua.c ./../../../../demos/rk3568_jailhouse/apps/ivshmem/
popd

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
