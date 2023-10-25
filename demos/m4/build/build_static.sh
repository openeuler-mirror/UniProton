git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

pushd ./../../../
python build.py m4
cp output/UniProton/lib/cortex_m4/* demos/m4/libs
cp output/libboundscheck/lib/cortex_m4/* demos/m4/libs
cp -r src/include/uapi/* demos/m4/include
cp build/uniproton_config/config_m4/prt_buildef.h demos/m4/include/
popd
