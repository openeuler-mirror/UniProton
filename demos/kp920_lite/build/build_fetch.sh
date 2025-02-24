pushd ../../../src/net/
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
