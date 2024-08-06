git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../../platform/libboundscheck/src
rm -rf libboundscheck

pushd ./../../../../
python build.py $1
cp output/UniProton/lib/$1/* boards/armv8/$1/libs
cp output/libboundscheck/lib/$1/* boards/armv8/$1/libs
cp -r output/libc boards/armv8/$1/include
cp -r src/include/uapi/* boards/armv8/$1/include
cp -r build/uniproton_config/config_armv8_hi3093/prt_buildef.h boards/armv8/$1/include/
popd
