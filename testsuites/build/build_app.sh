#!/bin/bash
set -e
export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-none-eabi-10-2020-q4-major

sim_flag=false
build_flag=""
LIB_RUN_TYPE="FPGA"

POSIX_APP=(
    "UniPorton_test_posix_time_interface" \
    "UniPorton_test_posix_thread_sem_interface" \
    "UniPorton_test_posix_thread_pthread_interface" \
    "UniPorton_test_posix_malloc_interface" \
    "UniPorton_test_posix_stdlib_interface" \
    "UniPorton_test_posix_ipc_interface" \
    "UniPorton_test_posix_signal_interface" \
    "UniPorton_test_proxy_posix_interface" \
    "UniPorton_test_posix_exit_interface"
    )

RHEALSTONE_APP=(
    "deadlock-break" \
    "interrupt-latency" \
    "message-latency" \
    "semaphore-shuffle" \
    "task-preempt" \
    "task-switch"
)

UART_APP=(
    "UniPorton_test_drivers_inode_interface" \
    "UniPorton_test_drivers_uart_interface" \
    "UniPorton_test_shell_interface"
)

MATH_APP=("UniPorton_test_posix_math_interface")

ALL_APP="${POSIX_APP[*]} ${RHEALSTONE_APP[*]} ${MATH_APP[*]} ${UART_APP[*]}"

for one_arg in $*; do
    if [[ $one_arg == "sim" || $one_arg == "SIM" ]]; then
        sim_flag=true
        ALL_APP="${POSIX_APP[*]} ${MATH_APP[*]}"
    elif [[ $one_arg == "posix" || $one_arg == "POSIX" ]]; then
        ALL_APP=${POSIX_APP[*]}
    elif [[ $one_arg == "rhealstone" || $one_arg == "RHEALSTONE" ]]; then
        ALL_APP=${RHEALSTONE_APP[*]}
    elif [[ $one_arg == "math" || $one_arg == "MATH" ]]; then
        ALL_APP=${MATH_APP[*]}
    elif [[ $one_arg == "uart" || $one_arg == "UART" ]]; then
        ALL_APP=${UART_APP[*]}
    else
        LIB_RUN_TYPE=$one_arg
    fi
done

pushd ../../
    python3 build.py m4 normal $LIB_RUN_TYPE
popd

echo "================================================"
echo "Will build ${ALL_APP}"
echo "================================================"

for one_app in ${ALL_APP[*]}
do
export APP=${one_app}
export TMP_DIR=$APP

if [[ $APP == "UniPorton_test_posix_math_interface" ]]; then
sed -i "s/CONFIG_LOSCFG_SHELL=y/# CONFIG_LOSCFG_SHELL=y/g" ../../build/uniproton_config/config_m4/defconfig
sed -i "s/CONFIG_OS_OPTION_NUTTX_VFS=y/# CONFIG_OS_OPTION_NUTTX_VFS=y/g" ../../build/uniproton_config/config_m4/defconfig
sed -i "s/CONFIG_OS_OPTION_DRIVER=y/# CONFIG_OS_OPTION_DRIVER=y/g" ../../build/uniproton_config/config_m4/defconfig
pushd ../../
    python3 build.py m4 normal $LIB_RUN_TYPE
popd
fi

if $sim_flag; then
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DSIM:STRING="_SIM_" -DLIB_RUN_TYPE:STRING=$LIB_RUN_TYPE -DCPU_TYPE:SRTING="m4"
else
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DLIB_RUN_TYPE:STRING=$LIB_RUN_TYPE -DCPU_TYPE:SRTING="m4"
fi
pushd $TMP_DIR
make $APP --trace
popd
if [[ $APP == "UniPorton_test_posix_math_interface" ]]; then
    all_math_app=$(ls $TMP_DIR/UniPorton_test_posix_math_*)
    for one_math in ${all_math_app}; do
        file_name=${one_math#*/*}
        cp $TMP_DIR/$file_name $file_name.elf
        $TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$file_name $file_name.bin
    done
    rm -rf $TMP_DIR
    if [[ $APP == "UniPorton_test_posix_math_interface" ]]; then
    sed -i "s/# CONFIG_LOSCFG_SHELL=y/CONFIG_LOSCFG_SHELL=y/g" ../../build/uniproton_config/config_m4/defconfig
    sed -i "s/# CONFIG_OS_OPTION_NUTTX_VFS=y/CONFIG_OS_OPTION_NUTTX_VFS=y/g" ../../build/uniproton_config/config_m4/defconfig
    sed -i "s/# CONFIG_OS_OPTION_DRIVER=y/CONFIG_OS_OPTION_DRIVER=y/g" ../../build/uniproton_config/config_m4/defconfig
    pushd ../../
    python3 build.py m4 normal $LIB_RUN_TYPE
    popd
    fi
elif [[ $APP == "UniPorton_test_posix_exit_interface" ]]; then
    all_exit_app=$(ls $TMP_DIR/UniPorton_test_posix_exit_*)
    for one_exit in ${all_exit_app}; do
        file_name=${one_exit#*/*}
        cp $TMP_DIR/$file_name $file_name.elf
        $TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$file_name $file_name.bin
    done
    rm -rf $TMP_DIR
else
    cp $TMP_DIR/$APP $APP.elf
    $TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$APP $APP.bin
    rm -rf $TMP_DIR
fi
done

echo "================================================"
echo "Build ${ALL_APP} done"
echo "================================================"
