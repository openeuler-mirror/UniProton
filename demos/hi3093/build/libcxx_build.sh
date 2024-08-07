CURRENT_DIR=$(pwd)
export GCC_PATH=$CURRENT_DIR/../component
export UNIPROTON_PATH=$CURRENT_DIR/../../..
export CXX_COMPILE_PATH=/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr

pushd $GCC_PATH
wget https://gcc.gnu.org/pub/gcc/releases/gcc-10.3.0/gcc-10.3.0.tar.gz
tar -xzvf gcc-10.3.0.tar.gz

patch -d ./gcc-10.3.0 -p1 < $UNIPROTON_PATH/demos/hi3093/component/libstdc++-uniproton.patch

CC_OPTION="-nostdinc -I $UNIPROTON_PATH/demos/hi3093/include/ -I $UNIPROTON_PATH/output/libc/include/"
CXX_OPTION="-I $GCC_PATH/gcc-10.3.0/libstdc++-v3/build/include/ -I $UNIPROTON_PATH/demos/hi3093/include/ -I $UNIPROTON_PATH/output/libc/include/ -I $CXX_COMPILE_PATH/lib64/gcc/aarch64-openeuler-linux-gnu/10.3.1/include -g -nostdlib -nostdinc -nostdinc++"
export SDKTARGETSYSROOT=/opt/openeuler/oecore-x86_64/sysroots/aarch64-openeuler-linux
export PATH="$PATH:/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr/bin"
export CC="aarch64-openeuler-linux-gcc  -mlittle-endian -fstack-protector-strong  -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=$SDKTARGETSYSROOT ${CC_OPTION}"
export CXX="aarch64-openeuler-linux-g++  -mlittle-endian -fstack-protector-strong  -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=$SDKTARGETSYSROOT ${CXX_OPTION}"
export CPP="aarch64-openeuler-linux-gcc -E  -mlittle-endian -fstack-protector-strong  -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=$SDKTARGETSYSROOT"
export AS="aarch64-openeuler-linux-as "
export LD="aarch64-openeuler-linux-ld   --sysroot=$SDKTARGETSYSROOT"
export STRIP=aarch64-openeuler-linux-strip
export RANLIB=aarch64-openeuler-linux-ranlib
export OBJCOPY=aarch64-openeuler-linux-objcopy
export OBJDUMP=aarch64-openeuler-linux-objdump
export READELF=aarch64-openeuler-linux-readelf
export AR=aarch64-openeuler-linux-ar
export NM=aarch64-openeuler-linux-nm
export TARGET_PREFIX=aarch64-openeuler-linux-
export CONFIGURE_FLAGS="--target=aarch64-openeuler-linux --host=aarch64-openeuler-linux --build=x86_64-linux --with-libtool-sysroot=$SDKTARGETSYSROOT"
export CFLAGS=" -O2 -pipe -g -feliminate-unused-debug-types "
export CXXFLAGS=" -O2 -pipe -g -feliminate-unused-debug-types "
export LDFLAGS="-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed  -Wl,--build-id=sha1 -Wl,-z,noexecstack -Wl,-z,relro,-z,now -static -Wl,--no-relax -Wl,-gc-sections"
export CPPFLAGS=""
export ARCH=arm64
export CROSS_COMPILE=aarch64-openeuler-linux-

cd ./gcc-10.3.0/libstdc++-v3
mkdir build && cd build
../configure --disable-multilib --disable-nls --disable-libstdcxx-pch --with-gxx-include-dir=include/ --enable-tls=no --disable-libstdcxx-verbose --enable-linux-futex=no --disable-shared --enable-clocale=generic --host=aarch64-openeuler-linux
make -j 8 >> make.log
make DESTDIR=$GCC_PATH/gcc-10.3.0/libstdc++-v3/build/dest install >> cxxmake.log

mkdir $UNIPROTON_PATH/demos/hi3093/component/libcxx
mkdir $UNIPROTON_PATH/demos/hi3093/component/libcxx/include && mkdir $UNIPROTON_PATH/demos/hi3093/component/libcxx/lib
cp ./dest/usr/local/lib64/* $UNIPROTON_PATH/demos/hi3093/component/libcxx/lib/
cp ./destinclude/aarch64-openeuler-linux/bits/* ./destinclude/bits/ && cp ./destinclude/aarch64-openeuler-linux/ext/* ./destinclude/ext/
rm -rf ./destinclude/aarch64-openeuler-linux
cp -r ./destinclude/* $UNIPROTON_PATH/demos/hi3093/component/libcxx/include/
popd
