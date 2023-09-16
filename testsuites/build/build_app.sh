set -e
pushd ../../
python3 build.py m4
popd

export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-none-eabi-10-2020-q4-major

sim_flag=false
build_flag=""

if [ $# -eq 1 ]; then
    if [[ $1 == "sim" || $1 == "SIM" ]]; then
        sim_flag=true
    else
        build_flag=$1
    fi
elif [ $# -eq 2 ]; then
    if [[ $1 == "sim" || $1 == "SIM" ]]; then
        sim_flag=true
        build_flag=$2
    elif [[ $2 == "sim" || $2 == "SIM" ]]; then
        sim_flag=true
        build_flag=$1
    else
        echo "[WARN]: parameter error, use default set"
    fi
fi

POSIX_APP=(
    "UniPorton_test_posix_time_interface" \
    "UniPorton_test_posix_thread_sem_interface" \
    "UniPorton_test_posix_thread_pthread_interface" \
    "UniPorton_test_posix_malloc_interface" \
    "UniPorton_test_posix_stdlib_interface" \
    "UniPorton_test_posix_ipc_interface"
)

RHEALSTONE_APP=(
    "deadlock-break" \
    "interrupt-latency" \
    "message-latency" \
    "semaphore-shuffle" \
    "task-preempt" \
    "task-switch"
)

MATH_APP=("UniPorton_test_posix_math_interface")

ALL_APP="${POSIX_APP[*]} ${RHEALSTONE_APP[*]} ${MATH_APP[*]}"
if ${sim_flag}; then
    ALL_APP="${POSIX_APP[*]} ${MATH_APP[*]}"
fi

if [[ $build_flag == "posix" || $build_flag == "POSIX" ]]; then
    ALL_APP=${POSIX_APP[*]}
elif [[ $build_flag == "rhealstone" || $build_flag == "RHEALSTONE" ]]; then
    ALL_APP=${RHEALSTONE_APP[*]}
elif [[ $build_flag == "math" || $build_flag == "MATH" ]]; then
    ALL_APP=${MATH_APP[*]}
fi

echo "================================================"
echo "Will build ${ALL_APP}"
echo "================================================"

for one_app in ${ALL_APP[*]}
do
export APP=${one_app}
export TMP_DIR=$APP
if $sim_flag; then
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DSIM:STRING="_SIM_"
else
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH
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
else
    cp $TMP_DIR/$APP $APP.elf
    $TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$APP $APP.bin
    rm -rf $TMP_DIR
fi
done

echo "================================================"
echo "Build ${ALL_APP} done"
echo "================================================"
