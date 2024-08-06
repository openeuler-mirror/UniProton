export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-none-eabi-10-2020-q4-major
export APP=helloworld
SIM=""

for one_arg in $*; do
    if [[ $one_arg == "sim" || $one_arg == "SIM" ]]; then
        SIM="_SIM_"
    elif [[ $one_arg == "shell" || $one_arg == "SHELL" ]]; then
        echo "[Info] shell not support qemu"
        export APP=shell
        SIM=""
    elif [[ $one_arg == "helloworld" || $one_arg == "HELLOWORLD" ]]; then
        export APP=helloworld
    elif [[ $one_arg == "stdio" || $one_arg == "STDIO" ]]; then
        export APP=stdio
    elif [[ $one_arg == "fatfs" || $one_arg == "FATFS" ]]; then
        export APP=fatfs
    else
        echo "[Error] Not support args - $one_arg"
        exit 1
    fi
done

export TMP_DIR=$APP

sh ./build_static.sh

cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DSIM:STRING=$SIM
pushd $TMP_DIR
make $APP
popd
cp $TMP_DIR/$APP $APP.elf
$TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$APP $APP.bin
rm -rf $TMP_DIR
