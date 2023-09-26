git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

pushd ./../../../
python build.py m4
cp output/UniProton/lib/cortex_m4/* demos/m4/libs
cp output/libboundscheck/lib/cortex_m4/* demos/m4/libs
popd
