if [ ! -d "../include" ]; then
    git clone https://gitee.com/openeuler/libboundscheck.git
    if [ $? != 0 ]; then
        tar -xvf libboundscheck.tar
    fi
    cp libboundscheck/include/* ../../../platform/libboundscheck/include
    cp libboundscheck/src/* ../../../platform/libboundscheck/src

    mkdir ../include
    cp -r ../../../src/include/uapi/* ../include
    cp libboundscheck/include/* ../include
    rm -rf libboundscheck
fi

pushd ./../../../

python3 build.py $1
cp output/UniProton/lib/$1/* demos/sd3403/libs
cp output/libboundscheck/lib/$1/* demos/sd3403/libs
cp -r output/libc demos/sd3403/include
cp -r src/include/uapi/* demos/sd3403/include
cp -r src/fs/include/* demos/sd3403/include
cp build/uniproton_config/config_armv8_sd3403/prt_buildef.h demos/$1/include/
popd
