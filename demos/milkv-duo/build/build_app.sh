#!/bin/bash

set -e

# absolute pathname for the dir with build.sh 
TOP_DIR=$(dirname $(realpath $0))

# absolute pathname for current dir that execute the build.sh
CUR_DIR=`pwd`

# absolute pathname for uniproton root dir
UNIPROTON_ROOT_DIR=$(realpath ${TOP_DIR}/../../..)

# absolute pathname for output dir
OUTPUT_DIR=${TOP_DIR}/output

####################### user config ##########################

# tmp output dir name
TMP_DIR=${TOP_DIR}/tmp_build
# app name
APPNAME="openamp"
# toolchain-prefix , must be a absolute pathname
TOOLCHAINPREFIX=/opt/buildtools/host-tools/gcc/riscv64-elf-x86_64/bin/riscv64-unknown-elf-

TOOLCHAINPATH=/opt/buildtools/host-tools/gcc/riscv64-elf-x86_64

############################################################

CC=${TOOLCHAINPREFIX}gcc

AS=${TOOLCHAINPREFIX}gcc

LD=${TOOLCHAINPREFIX}ld

OBJDUMP=${TOOLCHAINPREFIX}objdump

OBJCOPY=${TOOLCHAINPREFIX}objcopy

NM=${TOOLCHAINPREFIX}nm

TOOLCHAIN=$(realpath $(dirname $(dirname ${TOOLCHAINPREFIX})))

debug="no"

if [ debug == "yes" ]; then
    echo "TOP_DIR = ${TOP_DIR}"
    echo "CUR_DIR = ${CUR_DIR}"
    echo "UNIPROTON_ROOT_DIR = ${UNIPROTON_ROOT_DIR}"
    echo "OUTPUT_DIR = ${OUTPUT_DIR}"
    echo "TMP_DIR = ${TMP_DIR}"
    echo "APPNAME = ${APPNAME}"
    echo "CC = ${CC}"
    echo "AS = ${AS}"
    echo "LD = ${LD}"
    echo "OBJDUMP = ${OBJDUMP}"
    echo "OBJCOPY = ${OBJCOPY}"
    echo "NM = ${NM}"
    echo "TOOLCHAIN = ${TOOLCHAIN}"
fi

#bash build_fetch.sh

#bash build_static.sh

bash build_openamp.sh ${TOOLCHAIN}

if [ -n "$1" ]; then
    APPNAME="$1"
fi

if [ ${APPNAME} != "openamp" ]; then
    echo "unkown app ${APPNAME}"
    exit -1
fi


cmake -S .. -B ${TMP_DIR} -DAPPNAME:STRING=${APPNAME} \
	-DUNIPROTON_ROOT_DIR:STRING=${UNIPROTON_ROOT_DIR} \
	-DTOOLCHAINPREFIX:STRING=${TOOLCHAINPREFIX}\
	-DTOOLCHAINPATH:STRING=${TOOLCHAINPATH}

pushd ${TMP_DIR}
    make
popd

if [ ! -d "${OUTPUT_DIR}" ]; then
    mkdir -p ${OUTPUT_DIR}
fi

rm -rf ${OUTPUT_DIR}/${APPNAME}*

cp ${TMP_DIR}/${APPNAME} ${OUTPUT_DIR}/${APPNAME}.elf

${OBJDUMP} -d -f ${OUTPUT_DIR}/${APPNAME}.elf > ${OUTPUT_DIR}/${APPNAME}.asm

${OBJCOPY} -O binary ${OUTPUT_DIR}/${APPNAME}.elf ${OUTPUT_DIR}/${APPNAME}.bin

${NM} ${OUTPUT_DIR}/${APPNAME}.elf > ${OUTPUT_DIR}/${APPNAME}.map

rm -rf ${TMP_DIR}
