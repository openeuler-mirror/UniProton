#!/bin/bash

# use to pass tool chain path to cmakefile
export TOOLCHAIN_PATH=/opt/buildtools/riscv
# use to check_out demo  path
export APP_PATH=../../demos/riscv64virt
# use to creat tmp file 
export TMP_DIR=`pwd`/tmp
# uset to target out elf dir
export OUT_DIR=`pwd`

cleanup() {
	rm -rf ${TMP_DIR}
}
set -e
trap cleanup EXIT

POSIX_APP=(
    "UniProton_test_posix_stdlib_interface" \
    # "UniProton_test_posix_ipc_interface" \
    "UniProton_test_posix_string_interface" \
    "UniProton_test_posix_regex_interface" \
    "UniProton_test_posix_prng_interface"
    )

MATH_APP=("UniProton_test_posix_math_interface")

RHEALSTONE_APP=(
    "task-switch" \
    "task-preempt" \
    "message-latency" \
    "deadlock-break" \
    "semaphore-shuffle"
)


ALL_APP="${POSIX_APP[*]} ${RHEALSTONE_APP[*]} ${MATH_APP[*]}"

for one_arg in $*; do
    if [[ $one_arg == "posix" || $one_arg == "POSIX" ]]; then
        ALL_APP=${POSIX_APP[*]}
    elif [[ $one_arg == "rhealstone" || $one_arg == "RHEALSTONE" ]]; then
        ALL_APP=${RHEALSTONE_APP[*]}
    elif [[ $one_arg == "math" || $one_arg == "MATH" ]]; then
    	ALL_APP=${MATH_APP[*]}
    else
        echo 'argv error'
        exit 1
    fi
done
#check if kernel is built
if [ -f "${APP_PATH}/libs/libRV64VIRT.a" ];then
    echo "have built yet"
else
    pushd ${APP_PATH}/build
    bash build_static.sh
    popd
fi

#check if base rheal elf in rv has been built
if [ -d "${OUT_DIR}/baserheal" ];then
    echo "have baserheal yet"
else
    git clone https://gitee.com/Jer6y/baserheal.git ${OUT_DIR}/baserheal
fi

echo "================================================"
echo "Will build ${ALL_APP}"
echo "================================================"

for one_app in ${ALL_APP}
do
    cmake -S ${APP_PATH} -B ${TMP_DIR}  -DAPP:STRING=${one_app} -DTOOLCHAIN_PATH:STRING=$TOOLCHAIN_PATH
    pushd ${TMP_DIR}
    make ${one_app}
    popd
    cp ${TMP_DIR}/${one_app} ${OUT_DIR}/${one_app}_rv.elf
    rm -rf ${TMP_DIR}
done

echo "================================================"
echo "Build ${ALL_APP} done"
echo "================================================"

