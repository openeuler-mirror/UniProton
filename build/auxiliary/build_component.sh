set -e
DEFCONFIG=$5

# 编译到内核:libxml2,soem
# 编译到具体单板:ccl,eigen,boost
# 通过静态库链接:openamp,libmetal,libcxx,modbus,forte
# 为加快编译速度，重复编译场景下，单独编译成静态库的组件，只有第一次会编译和下载，后续不再编译和下载，使用首次编译的静态库
# 如果组件想重新编译，可以./build.sh clean命令清除首次编译得到的静态库
# 如果都是armv8平台，不同款型之间组件静态库可以共用。如果是从armv8平台切换到x86，则需要手动清除一次

#----------------------------------------------------------------------------------
# openamp组件编译,不想重新编,关闭对应宏开关即可
if [ $(grep -c "CONFIG_OS_OPTION_OPENAMP=y" "$DEFCONFIG") -gt 0 ] && [ ! -d "${HOME_PATH}/build/open-amp/output" ]
then
	# x86 和 armv8 平台对应的工具链不一样
	echo "######################### build metal #########################"
	pushd ${HOME_PATH}/build
	mkdir -p libmetal
	cd libmetal
	mkdir -p build
	cd build
	rm -rf *
	if [ "$2" = "armv8" ]; then
			echo "=========================armv8 libmetal======================="
			cmake ${HOME_PATH}/src/component/libmetal -DCMAKE_TOOLCHAIN_FILE=${HOME_PATH}/src/component/libmetal/cmake/platforms/uniproton_arm64_gcc.cmake -DTOOLCHAIN_PATH:STRING=$1 -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF -DKCONF_DIR=$4
	else
			echo "=========================x86_64 libmetal======================="
			cmake ${HOME_PATH}/src/component/libmetal -DCMAKE_TOOLCHAIN_FILE=${HOME_PATH}/src/component/libmetal/cmake/platforms/uniproton_x86_64_gcc.cmake -DTOOLCHAIN_PATH:STRING=$1 -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=OFF -DWITH_SHARED_LIB=OFF -DKCONF_DIR=$4
	fi

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
	if [ "$2" = "armv8" ]; then
			echo "=========================armv8 openamp======================="
			cmake ${HOME_PATH}/src/component/open-amp -DCMAKE_TOOLCHAIN_FILE=${HOME_PATH}/src/component/open-amp/cmake/platforms/uniproton_arm64_gcc.cmake -DTOOLCHAIN_PATH:STRING=$1 -DARCH_TYPE=$2 -DCPU_TYPE=$3 -DKCONF_DIR=$4
	else
			echo "=========================x86_64 openamp======================="
			cmake ${HOME_PATH}/src/component/open-amp -DCMAKE_TOOLCHAIN_FILE=${HOME_PATH}/src/component/open-amp/cmake/platforms/uniproton_x86_64_gcc.cmake -DTOOLCHAIN_PATH:STRING=$1 -DARCH_TYPE=$2 -DCPU_TYPE=$3 -DKCONF_DIR=$4
	fi

	make VERBOSE=1 DESTDIR=../output install
	if [ $? -ne 0 ];then
			echo "make openamp failed!"
			exit 1
	fi
	popd
fi

#----------------------------------------------------------------------------------
# cxx编译,编译时间较长,只编一次,不删除上次。该脚本仅支持armv8平台，x86已经编译完成
if [ $(grep -c "CONFIG_OS_SUPPORT_CXX=y" "$DEFCONFIG") -gt 0 ] && [ ! -d "${HOME_PATH}/build/libcxx/lib" ] && [ "$2" = "armv8" ]
then
	export GCC_PATH=$HOME_PATH/src/component
	export UNIPROTON_PATH=$HOME_PATH
	export CXX_COMPILE_PATH=/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr

	pushd $GCC_PATH
	CC_OPTION="-nostdinc -I $UNIPROTON_PATH/build/uniproton_config/config_$4/ -I $UNIPROTON_PATH/src/include/uapi/ -I $UNIPROTON_PATH/platform/libboundscheck/include/ -I $UNIPROTON_PATH/build/output/libc/include/"
	CXX_OPTION="-I $UNIPROTON_PATH/build/uniproton_config/config_$4/ -I $GCC_PATH/gcc-10.3.0/libstdc++-v3/build/include/ -I $UNIPROTON_PATH/src/include/uapi/ -I $UNIPROTON_PATH/platform/libboundscheck/include/ -I $UNIPROTON_PATH/build/output/libc/include/ -I $CXX_COMPILE_PATH/lib64/gcc/aarch64-openeuler-linux-gnu/10.3.1/include -g -nostdlib -nostdinc -nostdinc++"
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

	mkdir $UNIPROTON_PATH/build/libcxx
	mkdir $UNIPROTON_PATH/build/libcxx/include && mkdir $UNIPROTON_PATH/build/libcxx/lib
	cp ./dest/usr/local/lib64/* $UNIPROTON_PATH/build/libcxx/lib/
	cp ./destinclude/aarch64-openeuler-linux/bits/* ./destinclude/bits/ && cp ./destinclude/aarch64-openeuler-linux/ext/* ./destinclude/ext/
	rm -rf ./destinclude/aarch64-openeuler-linux
	cp -r ./destinclude/* $UNIPROTON_PATH/build/libcxx/include/
	popd
fi

#----------------------------------------------------------------------------------
# lua编译,目前仅arm支持配置lua,x86未验证，先不指定
if [ $(grep -c "CONFIG_OS_OPTION_LUA=y" "$DEFCONFIG") -gt 0 ] && [ ! -e "${HOME_PATH}/build/lua/lib/liblua.a" ] && [ "$2" = "armv8" ]
then
	export LUA_GCC_PATH=/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr/bin/aarch64-openeuler-linux-gcc
	pushd ${HOME_PATH}/build
	rm -rf lua
	mkdir -p lua
	cd lua
	mkdir -p include
	mkdir -p lib
	popd
	pushd ${HOME_PATH}/src/component/lua-5.3.4/src
	make posix
	make echo
	cp ./liblua.a ${HOME_PATH}/build/lua/lib/
	cp ./lua.h ${HOME_PATH}/build/lua/include/
	cp ./luaconf.h ${HOME_PATH}/build/lua/include/
	cp ./lualib.h ${HOME_PATH}/build/lua/include/
	cp ./lauxlib.h ${HOME_PATH}/build/lua/include/
	cp ./lua.hpp ${HOME_PATH}/build/lua/include/

	# 临时拷贝，用于排查lua依赖
	cp ./lprefix.h ${HOME_PATH}/build/lua/include/
	popd
fi

#----------------------------------------------------------------------------------
# modbus编译
if [ $(grep -c "CONFIG_OS_OPTION_MODBUS=y" "$DEFCONFIG") -gt 0 ] && [ ! -e "${HOME_PATH}/build/modbus/lib/libmodbus.a" ]
then
	echo "######################### build modbus #########################"
	pushd ${HOME_PATH}/build
	mkdir -p modbus
	cd modbus
	rm -rf *
	popd

	pushd ${HOME_PATH}/src/component/libmodbus
	./autogen.sh
	if [ "$2" = "armv8" ]; then
		echo "=========================armv8 modbus======================="
		./configure CC=$1/bin/aarch64-openeuler-linux-gnu-gcc  --prefix ${HOME_PATH}/build/modbus  --enable-static=yes --disable-tests CFLAGS="-g -march=armv8.2-a+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -funwind-tables -nostdlib -nostdinc" CPPFLAGS="-nostdinc -I ${HOME_PATH}/build/output/libc/include -I ${HOME_PATH}/platform/libboundscheck/include -I ${HOME_PATH}/src/include/uapi/ -I ${HOME_PATH}/build/uniproton_config/config_${4}/" CXX=/usr1/openeuler/gcc/openeuler_gcc_x86_64/bin/aarch64-openeuler-linux-gnu-g++ --host x86
	else
		echo "=========================x86_64 modbus======================="
		./configure CC=$1/bin/x86_64-openeuler-linux-gnu-gcc  --prefix ${HOME_PATH}/build/modbus  --enable-static=yes --disable-tests CFLAGS="-g -O2 -mcmodel=large -static -nostdlib -nostdinc -fno-builtin -funwind-tables -nostartfiles -nodefaultlibs -mpreferred-stack-boundary=3 -mno-3dnow -mno-avx -mno-red-zone -Wl,--build-id=none -fno-builtin -fno-PIE -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -fno-stack-protector -funsigned-char -fno-PIC -nostdinc++" CPPFLAGS="-nostdinc -I ${HOME_PATH}/build/output/libc/include -I ${HOME_PATH}/platform/libboundscheck/include -I ${HOME_PATH}/src/include/uapi/ -I ${HOME_PATH}/build/uniproton_config/config_${4}/" CXX=/usr1/openeuler/gcc/openeuler_gcc_x86_64/bin/x86_64-openeuler-linux-gnu-g++ --host x86
	fi
	make
	make install
	popd
fi

#----------------------------------------------------------------------------------
# forte编译依赖cxx
if [ $(grep -c "CONFIG_OS_SUPPORT_CXX=y" "$DEFCONFIG") -gt 0 ] && [ $(grep -c "CONFIG_OS_OPTION_FORTE=y" "$DEFCONFIG") -gt 0 ] && [ ! -e "${HOME_PATH}/build/build_forte/src/libforte-static.a" ] && [ "$2" = "x86_64" ]
then
	echo "######################### build forte #########################"
	rm -rf ${HOME_PATH}/build/build_forte
	mkdir ${HOME_PATH}/build/build_forte
	cmake -S ${HOME_PATH}/src/component/forte -B ${HOME_PATH}/build/build_forte/ -DCMAKE_TOOLCHAIN_FILE=${HOME_PATH}/src/component/forte/uniproton_x86_64_forte.cmake -DTOOLCHAIN_PATH=$1 -DFORTE_ARCHITECTURE=Posix -DFORTE_BUILD_STATIC_LIBRARY=ON -DFORTE_BUILD_EXECUTABLE=OFF -DFORTE_COM_ETH=ON -DFORTE_COM_FBDK=ON -DFORTE_COM_LOCAL=ON -DFORTE_TESTS=OFF -DFORTE_TESTS_INC_DIRS=${forte_boost_test_inc_dirs} -DFORTE_TESTS_LINK_DIRS=${forte_boost_test_inc_dirs} -DFORTE_MODULE_CONVERT=ON -DFORTE_MODULE_IEC61131=ON -DFORTE_MODULE_UTILS=ON -DKCONF_DIR=$4
	cmake --build ${HOME_PATH}/build/build_forte/ -- -j 8
	echo "######################### build forte end #########################" 
fi