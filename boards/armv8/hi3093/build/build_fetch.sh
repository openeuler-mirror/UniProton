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
cp UniProton-patch-for-libmetal.patch ./libmetal
cd libmetal
patch -p1 -d . < UniProton-patch-for-libmetal.patch
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
cp UniProton-patch-for-openamp.patch ./open-amp
cd open-amp
patch -p1 -d . < UniProton-patch-for-openamp.patch
fi
popd

pushd ../component
if [[ "${APP}" == "cxxTest" || "${APP}" == "eigenTest" ]] && [ ! -d "./boost_1_54_0" ]
then
echo "################# get boost #################"
rm -rf boost_1_54_0*
wget https://jaist.dl.sourceforge.net/project/boost/boost/1.54.0/boost_1_54_0.tar.bz2
tar -xjf boost_1_54_0.tar.bz2
cp UniProton-patch-for-boost_1_54_0.patch boost_1_54_0
cd boost_1_54_0
patch -p1 -d . < UniProton-patch-for-boost_1_54_0.patch
fi
popd

pushd ../component
if [[ "${APP}" == "eigenTest" && ! -d "./eigen-3.4.0" ]]
then
echo "################# get eigen #################"
rm -rf eigen-3.4.0*
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar -zxvf eigen-3.4.0.tar.gz
cp eigen-3.4.0/doc/examples/*cpp* ./../../../testsuites/eigen-test/
fi
popd

pushd ../component
if [[ "${APP}" == "UniPorton_test_libxml2_interface" && ! -d "./libxml2" ]]
then
echo "################# get libxml2 #################"
rm -rf ./libxml2
git clone https://gitee.com/src-openeuler/libxml2.git
cd ./libxml2
git checkout openEuler-22.03-LTS-SP2
cd ../
cp ./libxml2/libxml2-2.9.14.tar.xz ./
rm -rf ./libxml2
tar -xf libxml2-2.9.14.tar.xz
rm libxml2-2.9.14.tar.xz
mv ./libxml2-2.9.14 ./libxml2
cp ./../../../src/component/UniProton-patch-for-libxml2.patch ./libxml2
cd libxml2
patch -p1 -d . < UniProton-patch-for-libxml2.patch
fi
popd

pushd ../../../../src/component
if [[ "${APP}" == "libcclTest" && ! -d "./ccl" ]]
then
echo "################# get ccl #################"
rm -rf ./ccl
wget https://files.sbooth.org/ccl-0.1.1.tar.gz
tar -xf ccl-0.1.1.tar.gz
rm ccl-0.1.1.tar.gz
mv ccl-0.1.1 ccl
cp UniProton-patch-for-ccl.patch ./ccl
cd ./ccl
patch -p1 -d . < UniProton-patch-for-ccl.patch
fi
popd

pushd ../../../../src/net/
if [[ "${APP}" == "UniProton_soem_demo" && ! -d "./soem" ]]
then
echo "################# get soem #################"
rm -rf ./soem
git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
cd oee_archive
git sparse-checkout init --cone
git sparse-checkout set "soem"
cp soem/SOEM-1.4.0.tar.gz ../
cd ../
rm -rf oee_archive
tar -zxf SOEM-1.4.0.tar.gz
rm SOEM-1.4.0.tar.gz
mv SOEM-1.4.0 soem
cp UniProton-patch-for-soem.patch ./soem
cd ./soem
patch -p1 -d . < UniProton-patch-for-soem.patch
rm -rf ../../../testsuites/soemTest/test
cp -r test ../../../testsuites/soemTest/
fi
popd