#!/bin/bash

set -e 

TOOLCHAIN=$1

echo "######################### build metal #########################"
pushd .
if [ ! -d "./libmetal" ]
then
    mkdir -p libmetal
fi

cd libmetal

if [ ! -d "./build" ]
then
    mkdir -p build
fi

cd build

rm -rf *

cmake ../../../component/libmetal -DCMAKE_TOOLCHAIN_FILE=../../../component/libmetal/cmake/platforms/uniproton_riscv64_gcc.cmake -DTOOLCHAIN_PATH:STRING=${TOOLCHAIN} -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
        echo "make metal failed!"
        exit 1
fi
popd

cp ./libmetal/output/usr/local/lib/*.a ../libs
cp -rf ./libmetal/output/usr/local/include/metal ../include

echo "######################### build openamp #########################"
pushd .

if [ ! -d "./openamp" ]
then
    mkdir -p open-amp
fi

cd open-amp

if [ ! -d "./build" ]
then
    mkdir -p build
fi

cd build

rm -rf *

cmake ../../../component/open-amp -DCMAKE_TOOLCHAIN_FILE=../../../component/open-amp/cmake/platforms/uniproton_riscv64_gcc.cmake -DTOOLCHAIN_PATH:STRING=${TOOLCHAIN}
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
        echo "make openamp failed!"
        exit 1
fi
popd

cp    ./open-amp/output/usr/local/lib/*.a ../libs
cp -r ./open-amp/output/usr/local/include/openamp ../include
