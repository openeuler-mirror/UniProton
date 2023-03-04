export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-none-eabi-10-2020-q4-major
export APP=helloworld
export TMP_DIR=$APP

sh ./build_static.sh

cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH
pushd $TMP_DIR
make $APP
popd
cp $TMP_DIR/$APP $APP.elf
$TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$APP $APP.bin
rm -rf $TMP_DIR
