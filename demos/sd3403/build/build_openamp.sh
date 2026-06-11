echo "################# git clone libmetal #################"
pushd ../component
if [ -d "./libmetal" ] && [ -f "./libmetal/CMakeLists.txt" ]; then
    echo "libmetal source already exists, skip download."
else
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

echo "################# git clone openamp #################"
pushd ../component
if [ -d "./open-amp" ] && [ -f "./open-amp/CMakeLists.txt" ]; then
    echo "open-amp source already exists, skip download."
else
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

echo "######################### build metal #########################"
pushd .
mkdir -p libmetal
cd libmetal
mkdir -p build
cd build
rm -rf *
cmake ../../../component/libmetal -DCMAKE_TOOLCHAIN_FILE=../../../component/libmetal/cmake/platforms/uniproton_arm64_gcc.cmake -DTOOLCHAIN_PATH:STRING=$1 -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
	echo "make metal failed!"
	exit 1
fi
popd

echo "######################### build openamp #########################"         
pushd .
mkdir -p open-amp
cd open-amp
mkdir -p build
cd build
rm -rf *
cmake ../../../component/open-amp -DCMAKE_TOOLCHAIN_FILE=../../../component/open-amp/cmake/platforms/uniproton_arm64_gcc.cmake -DTOOLCHAIN_PATH:STRING=$1
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
        echo "make openamp failed!"
        exit 1
fi
popd

cp ./libmetal/output/usr/local/lib/*.a ../libs
cp ./open-amp/output/usr/local/lib/*.a ../libs
