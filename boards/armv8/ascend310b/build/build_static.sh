if [ ! -d "../include" ]; then
    git clone https://gitee.com/openeuler/libboundscheck.git
    if [ $? != 0 ]; then
        tar -xvf libboundscheck.tar
    fi
    cp libboundscheck/include/* ../../../../platform/libboundscheck/include
    cp libboundscheck/src/* ../../../../platform/libboundscheck/src

    mkdir ../include
    cp -r ../../../../src/include/uapi/* ../include
    cp libboundscheck/include/* ../include
    rm -rf libboundscheck
fi

pushd ./../../../../

python3 build.py $1
cp output/UniProton/lib/$1/* boards/armv8/ascend310b/libs
cp output/libboundscheck/lib/$1/* boards/armv8/ascend310b/libs
cp -r output/libc boards/armv8/ascend310b/include
cp -r src/include/uapi/* boards/armv8/ascend310b/include
cp build/uniproton_config/config_armv8_ascend310b/prt_buildef.h boards/armv8/$1/include/
popd
