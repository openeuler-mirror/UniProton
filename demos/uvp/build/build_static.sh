pushd ./../../../
python build.py $1
cp output/UniProton/lib/$1/* demos/uvp/libs
cp output/libboundscheck/lib/$1/* demos/uvp/libs
cp -r output/libc demos/uvp/include
cp build/uniproton_config/config_x86_uvp/prt_buildef.h demos/uvp/include
cp -r src/include/uapi/* demos/uvp/include
popd
