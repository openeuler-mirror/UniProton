echo "################# get libxml2 #################"
pushd ../../../src/component
rm -rf ./libxml2
# wget https://download.gnome.org/sources/libxml2/2.9/libxml2-2.9.14.tar.xz
git clone https://gitee.com/src-openeuler/libxml2.git
cd ./libxml2
git checkout openEuler-22.03-LTS-SP2
cd ../
cp ./libxml2/libxml2-2.9.14.tar.xz ./
rm -rf ./libxml2
tar -xf libxml2-2.9.14.tar.xz
rm libxml2-2.9.14.tar.xz
mv ./libxml2-2.9.14 ./libxml2
cp UniProton-patch-for-libxml2.patch ./libxml2
cd libxml2
patch -p1 -d . < UniProton-patch-for-libxml2.patch
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

echo "################# git clone ethercat #################"
pushd ../../../src/net/
rm -rf ./ethercat
git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
cd oee_archive
git sparse-checkout init --cone
git sparse-checkout set "igh-ethercat"
cp igh-ethercat/igh_master_20230720.tar.gz ../
cd ../
rm -rf oee_archive
tar -zxf igh_master_20230720.tar.gz
rm igh_master_20230720.tar.gz
cp UniProton-patch-for-igh.patch ./ethercat
cd ethercat
patch -p1 -d . < UniProton-patch-for-igh.patch
popd

echo "################# git clone libboundscheck #################"
git clone https://gitee.com/openeuler/libboundscheck.git
cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

echo "################# git clone libmetal #################"
pushd ../component
rm -rf ./libmetal*
git clone https://gitee.com/src-openeuler/libmetal.git
mv ./libmetal/libmetal-2022.10.0.tar.gz .
rm -rf ./libmetal
tar -zxf libmetal-2022.10.0.tar.gz
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
tar -zxf openamp-2022.10.1.tar.gz
mv ./openamp-2022.10.1 ./open-amp
cp UniProton-patch-for-openamp.patch ./open-amp
cd open-amp
patch -p1 -d . < UniProton-patch-for-openamp.patch
popd

echo "################# get ccl #################"
pushd ../../../src/component
rm -rf ./ccl
wget https://files.sbooth.org/ccl-0.1.1.tar.gz
tar -xf ccl-0.1.1.tar.gz
rm ccl-0.1.1.tar.gz
mv ccl-0.1.1 ccl
cp UniProton-patch-for-ccl.patch ./ccl
cd ./ccl
patch -p1 -d . < UniProton-patch-for-ccl.patch
popd

echo "################# git clone modbus #################"
pushd ../component
rm -rf ./libmodbus
git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
cd oee_archive
git sparse-checkout init --cone
git sparse-checkout set "libmodbus"
cp libmodbus/libmodbus-3.1.10.tar.gz ../
cd ../
rm -rf oee_archive
tar -zxf libmodbus-3.1.10.tar.gz
mv libmodbus-3.1.10 libmodbus
rm libmodbus-3.1.10.tar.gz
cp UniProton-patch-for-libmodbus.patch ./libmodbus
cd libmodbus
patch -p1 -d . < UniProton-patch-for-libmodbus.patch
popd

echo "################# git clone forte #################"
pushd ../component
rm -rf ./forte_2.0.1
git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
cd oee_archive
git sparse-checkout init --cone
git sparse-checkout set "forte"
cp forte/forte_2.0.1.tar.gz ../
cd ../
rm -rf oee_archive
tar -zxf forte_2.0.1.tar.gz
rm forte_2.0.1.tar.gz
cp UniProton-patch-for-forte.patch ./forte_2.0.1
cd forte_2.0.1
patch -p1 -d . < UniProton-patch-for-forte.patch
cd ../
rm -rf forte
mv forte_2.0.1 forte
popd