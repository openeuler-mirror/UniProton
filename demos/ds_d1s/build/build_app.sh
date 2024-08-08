if [ "$RISCV_NATIVE" = "true" ]; then
    export TOOLCHAIN_PATH="/usr"
    export TOOLCHAIN_PREFIX=""
else
    export TOOLCHAIN_PATH="/opt/buildtools/riscv"
    export TOOLCHAIN_PREFIX="riscv64-unknown-elf-"
fi

export APP=shell


for one_arg in $*; do
    if [[ $one_arg == "shell" ]]; then
        export APP=shell
    else
        echo "[Error] Not support args - $one_arg"
        exit 1
    fi
done
export TMP_DIR=$APP

# 这个是编译出内核的静态库
# 如果你只修改了app代码但是没有修改固件
# 注释掉他 否则一定要重新编译
# 根据你在开发的时候选择对应的策略
bash ./build_static.sh

cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH

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
