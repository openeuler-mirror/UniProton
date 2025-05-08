export TOOLCHAIN_PATH="/home/qzl/Tools/riscv-none-elf"
export TOOLCHAIN_PREFIX="riscv64-unknown-elf-"
export APP=shell
export TMP_DIR=shell

# 这个是编译出内核的静态库
# 如果你只修改了app代码但是没有修改固件
# 注释掉他 否则一定要重新编译
# 根据你在开发的时候选择对应的策略
bash ./build_static.sh

cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY

pushd $TMP_DIR
make $APP
popd
if [ ! -d "out"  ]; then
  mkdir out
fi 
cp ./$TMP_DIR/$APP ./out/$APP.elf
$TOOLCHAIN_PATH/bin/${TOOLCHAIN_PREFIX}objcopy -O binary ./out/$APP.elf ./out/$APP.bin
$TOOLCHAIN_PATH/bin/${TOOLCHAIN_PREFIX}objdump -D ./out/$APP.elf > ./out/$APP.asm
rm -rf $TMP_DIR
