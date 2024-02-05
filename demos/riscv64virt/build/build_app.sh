export TOOLCHAIN_PATH=/opt/buildtools/riscv
export APP=rv64virt
export TMP_DIR=$APP


bash ./build_static.sh
cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH

pushd $TMP_DIR
make $APP
popd
mkdir out
cp ./$TMP_DIR/$APP ./out/$APP.elf
$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objcopy -O binary ./out/$APP.elf ./out/$APP.bin
$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objdump -D ./out/$APP.elf > ./out/$APP.asm
rm -rf $TMP_DIR
