#!/bin/bash

set -e

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

#compile kernel
python build.py milkvduol

#copy lib to demo
cp output/UniProton/lib/milkvduol/* demos/milkv-duo/libs
cp output/libboundscheck/lib/milkvduol/* demos/milkv-duo/libs

#copy header file to demo
cp -r output/UniProton/include/*  	demos/milkv-duo/include
cp -r output/libc 			demos/milkv-duo/include
cp build/uniproton_config/config_riscv64_milkvduol/prt_buildef.h demos/milkv-duo/include/
popd
