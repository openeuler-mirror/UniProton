pushd ./../../../../
python build.py $1
cp output/UniProton/lib/$1/* boards/x86_64/uvp/libs
cp output/libboundscheck/lib/$1/* boards/x86_64/uvp/libs
cp -r output/libc boards/x86_64/uvp/include
cp build/uniproton_config/config_x86_uvp/prt_buildef.h boards/x86_64/uvp/include
cp -r src/include/uapi/* boards/x86_64/uvp/include
popd
