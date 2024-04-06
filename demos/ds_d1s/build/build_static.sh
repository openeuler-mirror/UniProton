git clone https://gitee.com/openeuler/libboundscheck.git

if [ ! -d "../include" ]; then
  mkdir ../include
fi

if [ ! -d "../libs" ]; then
  mkdir ../libs
fi

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src

rm -rf libboundscheck

pushd ./../../../
python build.py ds-d1s
cp  output/UniProton/lib/ds-d1s/* demos/ds_d1s/libs
cp  output/libboundscheck/lib/ds-d1s/* demos/ds_d1s/libs
cp -r src/include/uapi/* demos/ds_d1s/include
cp build/uniproton_config/config_riscv64_d1s/prt_buildef.h demos/ds_d1s/include/
popd
