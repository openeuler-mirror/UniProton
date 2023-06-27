pushd ../../
python3 build.py m4
popd

export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-none-eabi-10-2020-q4-major

ALL_APP=(
    "UniPorton_test_posix_time_interface" \
         "UniPorton_test_posix_thread_sem_interface" 
         "UniPorton_test_posix_thread_pthread_interface"
         )

for one_app in ${ALL_APP[*]}
do
export APP=${one_app}
export TMP_DIR=$APP
cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH
pushd $TMP_DIR
make $APP --trace
popd
cp $TMP_DIR/$APP $APP.elf
$TOOLCHAIN_PATH/bin/arm-none-eabi-objcopy -O binary $TMP_DIR/$APP $APP.bin
rm -rf $TMP_DIR
done