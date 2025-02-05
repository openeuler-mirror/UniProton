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
python build.py visionfive2
cp  output/UniProton/lib/visionfive2/* demos/visionfive2/libs
cp  output/libboundscheck/lib/visionfive2/* demos/visionfive2/libs
cp -r src/include/uapi/* demos/visionfive2/include
cp build/uniproton_config/config_riscv64_visionfive2/prt_buildef.h demos/visionfive2/include/
popd
