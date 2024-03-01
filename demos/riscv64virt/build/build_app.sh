export TOOLCHAIN_PATH=/opt/buildtools/riscv
export APP=hello_world

for one_arg in $*; do
    if [[ $one_arg == "hello_world" ]]; then
        export APP=hello_world
    elif [[ $one_arg == "rhealstone-task-switch" ]]; then
        export APP=rhealstone-task-switch
    elif [[ $one_arg == "rhealstone-task-preempt" ]]; then
        export APP=rhealstone-task-preempt
    elif [[ $one_arg == "rhealstone-message-latency" ]]; then
        export APP=rhealstone-message-latency
    elif [[ $one_arg == "rhealstone-deadlock-break" ]]; then
        export APP=rhealstone-deadlock-break
    elif [[ $one_arg == "rhealstone-semaphore-shuffle" ]]; then
        export APP=rhealstone-semaphore-shuffle
    else
        echo "[Error] Not support args - $one_arg"
        exit 1
    fi
done
export TMP_DIR=$APP

bash ./build_static.sh
cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH

pushd $TMP_DIR
make $APP
popd
if [ ! -d "out"  ]; then
  mkdir out
fi 
cp ./$TMP_DIR/$APP ./out/$APP.elf
$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objcopy -O binary ./out/$APP.elf ./out/$APP.bin
$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objdump -D ./out/$APP.elf > ./out/$APP.asm
rm -rf $TMP_DIR
