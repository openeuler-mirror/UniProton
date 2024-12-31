#!/bin/bash

set -e

pushd ../component
if [ ! -d "./libmetal" ]
then
echo "################# git clone libmetal #################"
rm -rf ./libmetal*
git clone https://gitee.com/src-openeuler/libmetal.git
mv ./libmetal/libmetal-2022.10.0.tar.gz .
rm -rf ./libmetal
tar -zxvf libmetal-2022.10.0.tar.gz
mv ./libmetal-2022.10.0 ./libmetal
cp UniProton-libmetal.patch ./libmetal
cd libmetal
patch -p1 -d . < UniProton-libmetal.patch
fi
popd

pushd ../component
if [ ! -d "./open-amp" ]
then
echo "################# git clone openamp #################"
rm -rf ./open-amp*
git clone https://gitee.com/src-openeuler/OpenAMP.git
mv ./OpenAMP/openamp-2022.10.1.tar.gz .
rm -rf ./OpenAMP
tar -zxvf openamp-2022.10.1.tar.gz
mv ./openamp-2022.10.1 ./open-amp
cp UniProton-openamp.patch ./open-amp
cd open-amp
patch -p1 -d . < UniProton-openamp.patch
fi
popd

