export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf
export APP=atlasa1
export TMP_DIR=$APP

sh ./build_static.sh $APP
sh ./build_openamp.sh $TOOLCHAIN_PATH

cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH
pushd $TMP_DIR
make $APP
popd
cp ./$TMP_DIR/$APP $APP.elf
$TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy -O binary ./$APP.elf $APP.bin
$TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump -D ./$APP.elf > $APP.asm
rm -rf $TMP_DIR
