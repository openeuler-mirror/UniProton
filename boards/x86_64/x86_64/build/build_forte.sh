echo "######################### build forte #########################"         
pushd .
rm -rf build_forte/
cmake -S ../component/forte -B build_forte/ -DCMAKE_TOOLCHAIN_FILE=$(dirname "$PWD")/component/forte/uniproton_x86_64_forte.cmake -DTOOLCHAIN_PATH=$1 -DFORTE_ARCHITECTURE=Posix -DFORTE_BUILD_STATIC_LIBRARY=ON -DFORTE_BUILD_EXECUTABLE=OFF -DFORTE_COM_ETH=ON -DFORTE_COM_FBDK=ON -DFORTE_COM_LOCAL=ON -DFORTE_TESTS=OFF -DFORTE_TESTS_INC_DIRS=${forte_boost_test_inc_dirs} -DFORTE_TESTS_LINK_DIRS=${forte_boost_test_inc_dirs} -DFORTE_MODULE_CONVERT=ON -DFORTE_MODULE_IEC61131=ON -DFORTE_MODULE_UTILS=ON
cmake --build build_forte/ -- -j 8
echo "######################### build forte end #########################" 
popd

cp ./build_forte/src/libforte-static.a ../libs