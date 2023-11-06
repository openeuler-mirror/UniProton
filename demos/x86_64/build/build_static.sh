echo "################# git clone ethercat #################"
pushd ../../../src/net/
git clone https://gitlab.com/Tim-Tianyu/ethercat.git
cd ethercat
git fetch origin
git checkout -b igh-23.09 origin/igh-23.09
cd ../
popd

git clone https://gitee.com/openeuler/libboundscheck.git

cp libboundscheck/include/* ../../../platform/libboundscheck/include
cp libboundscheck/include/* ../include
cp libboundscheck/src/* ../../../platform/libboundscheck/src
rm -rf libboundscheck

cp x86_64-patch-for-UniProton.patch ./../../../
pushd ./../../../
patch -N -p1 -d . < x86_64-patch-for-UniProton.patch
python build.py $1
cp output/UniProton/lib/$1/* demos/$1/libs
cp output/libboundscheck/lib/$1/* demos/$1/libs
cp -r output/libc demos/$1/include
cp output/linux/lib/$1/* demos/$1/libs
cp output/ethercat/lib/$1/* demos/$1/libs
cp -r src/include/uapi/* demos/$1/include
cp -r output/linux/include/* demos/$1/include
cp -r output/ethercat/include/* demos/$1/include
popd
