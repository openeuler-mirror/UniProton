#!/bin/bash
# Copyright © Huawei Technologies Co., Ltd. 2010-2020. All rights reserved.

set -e 
if [ $# != 3 ] ; then 
    echo "USAGE: $0 AR_TOOL_PATH CK_LIB_PATH CK_LIB_SUFFIX" 
    exit 1;
fi


AR_TOOL_PATH="$1"
CK_LIB_PATH="$2"
CK_LIB_SUFFIX="$3"
file=lib"${CK_LIB_SUFFIX}"


if [ "${CPU_TYPE}" = "m4" ] ; then
    ARNAME=arm-none-eabi-ar ; OBJCOPYNAME=arm-none-eabi-objcopy;
else
    ARNAME=ar; OBJCOPYNAME=objcopy;
fi

sleep 2
pushd "$CK_LIB_PATH"
##为什么不加这一行要报错
if [ "${CPU_TYPE}" = "m4" ] ; then
    [ -n tmp_"${file}" ] && rm -rf tmp_"${file}" 
fi
mkdir tmp_"${file}"
mv "$file" tmp_"${file}"/

pushd tmp_"${file}"
"$AR_TOOL_PATH"/"${ARNAME}" -x "$file"
# 删除某变量指定的目录下所有文件。
# 通过对变量${FILE_PATH}进行判断，当${FILE_PATH}为空时，不会错误删除根目录下的文件。
[ -n "${file}" ] && rm -rf "${file}"
if [ "${CPU_TYPE}" = "m4" ] ; then  
    find . -name '*.s.o'| awk -F "." '{print $2}'|xargs -I'{}' mv ./{}.s.o ./{}.o
    find . -name '*.S.o'| awk -F "." '{print $2}'|xargs -I'{}' mv ./{}.S.o ./{}.o
    find . -name '*.c.o'| awk -F "." '{print $2}'|xargs -I'{}' mv ./{}.c.o ./{}.o
    
    for i in $(ls *.o); 
    do 
        if [ -f "${i}" ]  ; then
            "${AR_TOOL_PATH}"/"${OBJCOPYNAME}" --remove-section=.comment -I elf32-littlearm -O elf32-littlearm "${i}" "${i}".oooo
            "${AR_TOOL_PATH}"/"${OBJCOPYNAME}" --strip-unneeded "${i}".oooo
            mv "${i}".oooo "${i}"
        fi
    done
fi

"$AR_TOOL_PATH"/"${ARNAME}" -r -D "$file" *.o
mv "$file" ../
popd

# 删除某变量指定的目录下所有文件。
# 通过对变量${FILE_PATH}进行判断，当${FILE_PATH}为空时，不会错误删除根目录下的文件。
[ -n tmp_"${file}" ] && rm -rf tmp_"${file}"
popd


exit 0