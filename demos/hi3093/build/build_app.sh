export TOOLCHAIN_PATH=/opt/buildtools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf

if [ "$1" != "" ]
then
    export APP=$1
else
    export APP=hi3093
fi

if [[ "${APP}" == "cxxTest" || "${APP}" == "eigenTest" ]]
then
    export CXX_TOOLCHAIN_PATH=/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr
fi

export TMP_DIR=$APP

sh ./build_fetch.sh
sh ./build_static.sh hi3093
sh ./build_openamp.sh $TOOLCHAIN_PATH

if [[ "${APP}" == "cxxTest" && ! -d "./../component/libcxx" ]]
then
    sh ./libcxx_build.sh
fi

if [[ "${APP}" == "cxxTest" || "${APP}" == "eigenTest" ]]
then
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCXX_TOOLCHAIN_PATH:STRING=$CXX_TOOLCHAIN_PATH -DCPU_TYPE:SRTING="hi3093"
else
    cmake -S .. -B $TMP_DIR -DAPP:STRING=$APP -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH -DCPU_TYPE:SRTING="hi3093"
fi

pushd $TMP_DIR
make $APP
popd
cp ./$TMP_DIR/$APP $APP.elf

if [[ "${APP}" == "cxxTest" || "${APP}" == "eigenTest" ]]
then
    python ./../../../bin_helper.py -f ./$APP.elf --nocopy
    $CXX_TOOLCHAIN_PATH/bin/aarch64-openeuler-linux-objcopy -O binary ./$APP.elf $APP.bin
    $CXX_TOOLCHAIN_PATH/bin/aarch64-openeuler-linux-objdump -D ./$APP.elf > $APP.asm
else
    $TOOLCHAIN_PATH/bin/aarch64-none-elf-objcopy -O binary ./$APP.elf $APP.bin
    $TOOLCHAIN_PATH/bin/aarch64-none-elf-objdump -D ./$APP.elf > $APP.asm
fi

rm -rf $TMP_DIR
