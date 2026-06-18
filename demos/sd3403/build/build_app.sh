# UniPorton_test_proxy_posix_interface sd3403 UniPorton_test_log_interface UniPorton_test_posix_time_interface UniProton_uros_demo
# deadlock-break interrupt-latency message-latency semaphore-shuffle task-preempt task-switch fs mbedtls cmsis zlib
export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf

if [ "$1" != "" ]
then
    export APP=$1
else
    export APP=sd3403
fi

export TMP_DIR=$APP

# clean old build artifacts
rm -f ../libs/*.a ../libs/*.lib
rm -rf ../../../output ./libmetal ./open-amp

sh ./build_fetch.sh
sh ./build_static.sh sd3403
DEFCONFIG=../../../build/uniproton_config/config_armv8_sd3403/defconfig
if grep -q "CONFIG_OS_OPTION_OPENAMP=y" "$DEFCONFIG"; then
    sh ./build_openamp.sh $TOOLCHAIN_PATH
fi

echo "cmake start"
cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="sd3403" -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
echo "cmake end"

pushd $TMP_DIR
make $APP
popd
cp ./$TMP_DIR/$APP $APP.elf
$TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy -O binary ./$APP.elf $APP.bin
$TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump -D ./$APP.elf > $APP.asm
rm -rf $TMP_DIR