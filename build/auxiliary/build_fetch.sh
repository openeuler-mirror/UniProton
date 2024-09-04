set -e
DEFCONFIG=$1
ARCH_TYPE=$2
# libxml2组件下载
if grep -q "CONFIG_OS_SUPPORT_LIBXML2=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./libxml2" ]
    then
        echo "################# get libxml2 #################"
        git clone https://gitee.com/src-openeuler/libxml2.git
        cd ./libxml2
        git checkout openEuler-22.03-LTS-SP2
        cd ../
        cp ./libxml2/libxml2-2.9.14.tar.xz ./
        rm -rf ./libxml2
        tar -xf libxml2-2.9.14.tar.xz
        rm libxml2-2.9.14.tar.xz
        mv ./libxml2-2.9.14 ./libxml2
        cp ./patch/UniProton-patch-for-libxml2.patch ./libxml2
        cd libxml2
        patch -p1 -d . < UniProton-patch-for-libxml2.patch
    fi
fi

# openamp组件下载
if grep -q "CONFIG_OS_OPTION_OPENAMP=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./libmetal" ]
    then
        echo "################# git clone libmetal #################"
        git clone https://gitee.com/src-openeuler/libmetal.git
        mv ./libmetal/libmetal-2022.10.0.tar.gz .
        rm -rf ./libmetal
        tar -zxf libmetal-2022.10.0.tar.gz
        rm -f libmetal-2022.10.0.tar.gz
        mv ./libmetal-2022.10.0 ./libmetal
        cp ./patch/UniProton-patch-for-libmetal.patch ./libmetal
        cd libmetal
        patch -p1 -d . < UniProton-patch-for-libmetal.patch
    fi
    popd

    pushd ${HOME_PATH}/src/component
    if [ ! -d "./open-amp" ]
    then
        echo "################# git clone openamp #################"
        git clone https://gitee.com/src-openeuler/OpenAMP.git
        mv ./OpenAMP/openamp-2022.10.1.tar.gz .
        rm -rf ./OpenAMP
        tar -zxf openamp-2022.10.1.tar.gz
        rm -f openamp-2022.10.1.tar.gz
        mv ./openamp-2022.10.1 ./open-amp
        cp ./patch/UniProton-patch-for-openamp.patch ./open-amp
        cd open-amp
        patch -p1 -d . < UniProton-patch-for-openamp.patch
    fi
    popd
fi

# cxx和boost库下载
if grep -q "CONFIG_OS_SUPPORT_CXX=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./gcc-10.3.0" ] && [ "${ARCH_TYPE}" = "armv8" ]
    then
        echo "################# get cxx support x86#################"
        wget https://gcc.gnu.org/pub/gcc/releases/gcc-10.3.0/gcc-10.3.0.tar.gz
        tar -zxf gcc-10.3.0.tar.gz
        patch -d ./gcc-10.3.0 -p1 < ./patch/libstdc++-uniproton.patch
    fi
    popd

    pushd ${HOME_PATH}/src/component
    if [ ! -d "./libcxx" ] && [ "${ARCH_TYPE}" = "x86_64" ]
    then
        echo "################# get cxx support x86 #################"
        git clone https://gitee.com/zuyiwen/libcxx
    fi
    popd

    pushd ${HOME_PATH}/src/component
    if [ ! -d "./boost_1_54_0" ]
    then
        echo "################# get boost #################"
        wget https://jaist.dl.sourceforge.net/project/boost/boost/1.54.0/boost_1_54_0.tar.bz2
        tar -xjf boost_1_54_0.tar.bz2
        cp ./patch/UniProton-patch-for-boost_1_54_0.patch boost_1_54_0
        cd boost_1_54_0
        patch -p1 -d . < UniProton-patch-for-boost_1_54_0.patch
    fi
    popd
fi

# 安全函数库下载
PATH_TO_CHECK="${HOME_PATH}/platform/libboundscheck/src"
FILE_PATTERN="*.c"  # 匹配所有.c文件

if [[ $(find "$PATH_TO_CHECK" -maxdepth 1 -name "$FILE_PATTERN" | wc -l) -eq 0 ]]; then
    git clone https://gitee.com/openeuler/libboundscheck.git
    cp libboundscheck/include/* ${HOME_PATH}/platform/libboundscheck/include
    cp libboundscheck/src/* ${HOME_PATH}/platform/libboundscheck/src
    rm -rf libboundscheck
fi

# eigen库下载
if grep -q "CONFIG_OS_SUPPORT_EIGEN=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./eigen-3.4.0" ]
    then
        echo "################# get eigen #################"
        wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
        tar -zxf eigen-3.4.0.tar.gz
        rm -f eigen-3.4.0.tar.gz
        cp eigen-3.4.0/doc/examples/*cpp* ${HOME_PATH}/testsuites/eigen-test/
    fi
    popd
fi

# ccl组件下载
if grep -q "CONFIG_OS_SUPPORT_CCL=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./ccl" ]
    then
        echo "################# get ccl #################"
        # rm -rf ./ccl
        wget https://files.sbooth.org/ccl-0.1.1.tar.gz
        tar -xf ccl-0.1.1.tar.gz
        rm ccl-0.1.1.tar.gz
        mv ccl-0.1.1 ccl
        cp ./patch/UniProton-patch-for-ccl.patch ./ccl
        cd ./ccl
        patch -p1 -d . < UniProton-patch-for-ccl.patch
    fi
    popd
fi

# soem组件下载
if grep -q "CONFIG_OS_SUPPORT_SOEM=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./soem" ]
    then
        echo "################# get soem #################"
        # rm -rf ./soem
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
        cp ./patch/UniProton-patch-for-soem.patch ./soem
        cd ./soem
        patch -p1 -d . < UniProton-patch-for-soem.patch
        rm -rf ${HOME_PATH}/testsuites/soemTest/test
        cp -r test ${HOME_PATH}/testsuites/soemTest/
    fi
    popd
fi

# ethercat下载
if grep -q "CONFIG_OS_SUPPORT_IGH_ETHERCAT=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/net
    if [ ! -d "./ethercat" ]
    then
        echo "################# get ethercat #################"
        git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
        cd oee_archive
        git sparse-checkout init --cone
        git sparse-checkout set "igh-ethercat"
        cp igh-ethercat/igh_master_20230720.tar.gz ../
        cd ../
        rm -rf oee_archive
        tar -zxf igh_master_20230720.tar.gz
        rm igh_master_20230720.tar.gz
        cp ./UniProton-patch-for-igh.patch ./ethercat
        cd ethercat
        patch -p1 -d . < UniProton-patch-for-igh.patch
    fi
    popd
fi

# modbus下载
if grep -q "CONFIG_OS_OPTION_MODBUS=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./libmodbus" ]
    then
        echo "################# get modbus #################"
        git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
        cd oee_archive
        git sparse-checkout init --cone
        git sparse-checkout set "libmodbus"
        cp libmodbus/libmodbus-3.1.10.tar.gz ../
        cd ../
        rm -rf oee_archive
        tar -zxf libmodbus-3.1.10.tar.gz
        mv libmodbus-3.1.10 libmodbus
        rm libmodbus-3.1.10.tar.gz
        cp ./patch/UniProton-patch-for-libmodbus.patch ./libmodbus
        cd libmodbus
        patch -p1 -d . < UniProton-patch-for-libmodbus.patch
    fi
    popd
fi

# forte下载
if grep -q "CONFIG_OS_OPTION_FORTE=y" "$DEFCONFIG"; then
    pushd ${HOME_PATH}/src/component
    if [ ! -d "./forte" ]
    then
        echo "################# get forte #################"
        git clone --depth=1 --filter=blob:none --sparse https://gitee.com/openeuler/oee_archive.git
        cd oee_archive
        git sparse-checkout init --cone
        git sparse-checkout set "forte"
        cp forte/forte_2.0.1.tar.gz ../
        cd ../
        rm -rf oee_archive
        tar -zxf forte_2.0.1.tar.gz
        rm forte_2.0.1.tar.gz
        cp ./patch/UniProton-patch-for-forte.patch ./forte_2.0.1
        cd forte_2.0.1
        patch -p1 -d . < UniProton-patch-for-forte.patch
        cd ../
        rm -rf forte
        mv forte_2.0.1 forte
    fi
    popd
fi