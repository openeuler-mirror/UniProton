if [ "$RISCV_NATIVE" = "true" ]; then
    export TOOLCHAIN_PATH="/usr"
    export TOOLCHAIN_PREFIX=""
else
    export TOOLCHAIN_PATH="/opt/buildtools/riscv"
    export TOOLCHAIN_PREFIX="riscv64-unknown-elf-"
fi

export APP=hello_world


for one_arg in $*; do
    if [[ $one_arg == "hello_world" ]]; then
        export APP=hello_world
    elif [[ $one_arg == "shell" ]]; then
	export APP=shell
    elif [[ $one_arg == "task-switch" || $one_arg == "rhealstone-task-switch" ]]; then
        export APP=task-switch
    elif [[ $one_arg == "task-preempt" || $one_arg == "rhealstone-task-preempt" ]]; then
        export APP=task-preempt
    elif [[ $one_arg == "message-latency" || $one_arg == "rhealstone-message-latency" ]]; then
        export APP=message-latency
    elif [[ $one_arg == "deadlock-break" || $one_arg == "rhealstone-deadlock-break" ]]; then
        export APP=deadlock-break
    elif [[ $one_arg == "semaphore-shuffle" || $one_arg == "rhealstone-semaphore-shuffle" ]]; then
        export APP=semaphore-shuffle
    elif [[ $one_arg == "UniProton_test_posix_math_interface" ]]; then
	export APP=UniProton_test_posix_math_interface
    elif [[ $one_arg == "UniProton_test_posix_stdlib_interface" ]]; then
	export APP=UniProton_test_posix_stdlib_interface
    elif [[ $one_arg == "UniProton_test_posix_ipc_interface" ]]; then
	export APP=UniProton_test_posix_ipc_interface
    elif [[ $one_arg == "UniProton_test_posix_string_interface" ]]; then
	export APP=UniProton_test_posix_string_interface
    elif [[ $one_arg == "UniProton_test_posix_regex_interface" ]]; then
	export APP=UniProton_test_posix_regex_interface
    elif [[ $one_arg == "UniProton_test_posix_prng_interface" ]]; then
	export APP=UniProton_test_posix_prng_interface
    elif [[ $one_arg == "rpmsglite_env_test" ]]; then
        export APP=rpmsglite_env_test
    elif [[ $one_arg == "rpmsglite_test_master" ]]; then
	export APP=rpmsglite_test_master
    elif [[ $one_arg == "rpmsglite_test_slave" ]]; then
	export APP=rpmsglite_test_slave
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
