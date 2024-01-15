#!/bin/bash
set -e

POSIX_APP=(
    "UniPorton_test_proxy_posix_interface" \
    "UniPorton_test_libxml2_interface"
    )

RHEALSTONE_APP=(
    "deadlock-break" \
    "interrupt-latency" \
    "message-latency" \
    "semaphore-shuffle" \
    "task-preempt" \
    "task-switch"
)

ALL_APP="${POSIX_APP[*]} ${RHEALSTONE_APP[*]}"

for one_arg in $*; do
    if [[ $one_arg == "posix" || $one_arg == "POSIX" ]]; then
        ALL_APP=${POSIX_APP[*]}
    elif [[ $one_arg == "rhealstone" || $one_arg == "RHEALSTONE" ]]; then
        ALL_APP=${RHEALSTONE_APP[*]}
    else
        echo 'argv error'
        exit 1
    fi
done

echo "================================================"
echo "Will build ${ALL_APP}"
echo "================================================"

cd ../../demos/x86_64/build/

for one_app in ${ALL_APP[*]}
do
sh ./build_app.sh ${one_app}
cp ./${one_app}.bin ../../../testsuites/build/
rm -rf ./${one_app}.bin
rm -rf ./${one_app}.asm
rm -rf ./${one_app}.elf
done

echo "================================================"
echo "Build ${ALL_APP} done"
echo "================================================"
