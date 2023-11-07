git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

pushd ./../../../
python build.py $1
cp output/UniProton/lib/$1/* demos/$1/libs
cp output/libboundscheck/lib/$1/* demos/$1/libs
cp -r src/include/uapi/* demos/$1/include
cp build/uniproton_config/config_armv8_kp920/prt_buildef.h demos/$1/include/
popd
