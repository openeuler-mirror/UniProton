echo "######################### build lwip #########################"
pushd .
mkdir -p lwip
cd lwip
mkdir -p build
cd build
rm -rf *
cmake ../../../component/lwip -DTOOLCHAIN_PATH=$1
make VERBOSE=1 DESTDIR=../output install
if [ $? -ne 0 ];then
	echo "make lwip failed!"
	exit 1
fi
popd

cp ./lwip/output/lib/uvp_x86/*.a ../libs