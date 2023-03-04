git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

pushd ./../../../
python build.py m4
cp output/UniProton/lib/cortex_m4/* demos/helloworld/libs
cp output/libboundscheck/lib/cortex_m4/* demos/helloworld/libs
cp -r src/include/uapi/* demos/helloworld/include
popd
