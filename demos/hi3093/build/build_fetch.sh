echo "################# git clone libmetal #################"
pushd ../component
rm -rf ./libmetal*
git clone https://gitee.com/src-openeuler/libmetal.git
mv ./libmetal/libmetal-2022.10.0.tar.gz .
rm -rf ./libmetal
tar -zxvf libmetal-2022.10.0.tar.gz
mv ./libmetal-2022.10.0 ./libmetal
cp UniProton-patch-for-libmetal.patch ./libmetal
cd libmetal
patch -p1 -d . < UniProton-patch-for-libmetal.patch
popd

echo "################# git clone openamp #################"
pushd ../component
rm -rf ./open-amp*
git clone https://gitee.com/src-openeuler/OpenAMP.git
mv ./OpenAMP/openamp-2022.10.1.tar.gz .
rm -rf ./OpenAMP
tar -zxvf openamp-2022.10.1.tar.gz
mv ./openamp-2022.10.1 ./open-amp
cp UniProton-patch-for-openamp.patch ./open-amp
cd open-amp
patch -p1 -d . < UniProton-patch-for-openamp.patch
popd

echo "################# get boost #################"
pushd ../component
rm -rf boost_1_54_0
wget https://jaist.dl.sourceforge.net/project/boost/boost/1.54.0/boost_1_54_0.tar.bz2
tar -xjf boost_1_54_0.tar.bz2
cp UniProton-patch-for-boost_1_54_0.patch boost_1_54_0
cd boost_1_54_0
patch -p1 -d . < UniProton-patch-for-boost_1_54_0.patch
popd

echo "################# get eigen #################"
pushd ../component
rm -rf eigen-3.4.0
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar -zxvf eigen-3.4.0.tar.gz
cp eigen-3.4.0/doc/examples/*cpp* ./../../../testsuites/eigen-test/
popd
