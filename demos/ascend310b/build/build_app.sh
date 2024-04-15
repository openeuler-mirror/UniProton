# UniPorton_test_proxy_posix_interface  ascend310b
export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf
export APP=ascend310b
export TMP_DIR=$APP

sh ./build_static.sh ascend310b
DEFCONFIG=../../../build/uniproton_config/config_armv8_ascend310b/defconfig
if grep -q "CONFIG_OS_OPTION_OPENAMP=y" "$DEFCONFIG"; then
    sh ./build_openamp.sh $TOOLCHAIN_PATH
fi

echo "cmake start"
cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="ascend310b"
echo "cmake end"

pushd $TMP_DIR
make $APP
popd
cp ./$TMP_DIR/$APP $APP.elf
$TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy -O binary ./$APP.elf $APP.bin
$TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump -D ./$APP.elf > $APP.asm
rm -rf $TMP_DIR