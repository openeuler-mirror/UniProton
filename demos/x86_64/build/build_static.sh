pushd ./../../../
python build.py $1
cp output/UniProton/lib/$1/* demos/$1/libs
cp output/libboundscheck/lib/$1/* demos/$1/libs
cp -r output/libc demos/$1/include
cp build/uniproton_config/config_x86_64/prt_buildef.h demos/x86_64/include

if [ -d "output/linux/include" ] ; then
    cp -r output/linux/include/* demos/$1/include
fi

if [ -d "output/ethercat/include" ] ; then
    cp -r output/ethercat/include/* demos/$1/include
fi

cp -r src/include/uapi/* demos/$1/include
popd
