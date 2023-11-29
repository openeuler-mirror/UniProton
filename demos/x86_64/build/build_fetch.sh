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

echo "################# git clone libcxx #################"
pushd ../component
git clone https://gitee.com/zuyiwen/libcxx.git
popd
