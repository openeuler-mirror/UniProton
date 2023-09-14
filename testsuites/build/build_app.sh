#!/bin/bash

sim_flag=false
build_flag=""
LIB_RUN_TYPE="FPGA"

for one in $* ; do
    if [[ ${one} == "sim" ||  ${one} == "SIM" ]]; then
        sim_flag=true
    elif [[ ${one} == "posix" || ${one} == "POSIX" ]]; then
        build_flag=${one}
    else
        LIB_RUN_TYPE=$1
    fi
done

pushd ../../
python3 build.py m4 normal $LIB_RUN_TYPE
popd

export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-none-eabi-10-2020-q4-major

POSIX_APP=(
    "UniPorton_test_posix_time_interface" \
         "UniPorton_test_posix_thread_sem_interface" \
         "UniPorton_test_posix_thread_pthread_interface" \
         "UniPorton_test_posix_signal_interface" \
         "UniPorton_test_proxy_posix_interface" \
         "UniPorton_test_posix_exit_interface"
         )

ALL_APP="${POSIX_APP[*]}"

if [[ $build_flag == "posix" || $build_flag == "POSIX" ]]; then
    ALL_APP=${POSIX_APP[*]}
fi

echo "================================================"
echo "Will build ${ALL_APP}"
echo "================================================"

for one_app in ${ALL_APP[*]}
do
export APP=${one_app}
export TMP_DIR=$APP

if $sim_flag; then
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DSIM:STRING="_SIM_" -DLIB_RUN_TYPE:STRING=$LIB_RUN_TYPE -DCPU_TYPE:SRTING="m4"
else
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DLIB_RUN_TYPE:STRING=$LIB_RUN_TYPE -DCPU_TYPE:SRTING="m4"
fi
pushd $TMP_DIR
make $APP --trace
popd
if [[ $APP == "UniPorton_test_posix_exit_interface" ]]; then
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