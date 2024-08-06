echo "######################### build modbus #########################"
pushd .
mkdir -p modbus
cd modbus
rm -rf *
popd

pushd ../component/libmodbus
./autogen.sh
./configure CC=$1/bin/x86_64-openeuler-linux-gnu-gcc  --prefix $(dirname $(dirname "$PWD"))/build/modbus  --enable-static=yes --disable-tests CFLAGS="-g -O2 -mcmodel=large -static -nostdlib -nostdinc -fno-builtin -funwind-tables -nostartfiles -nodefaultlibs -mpreferred-stack-boundary=3 -mno-3dnow -mno-avx -mno-red-zone -Wl,--build-id=none -fno-builtin -fno-PIE -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -fno-stack-protector -funsigned-char -fno-PIC -nostdinc++" CPPFLAGS="-nostdinc -I /home/openeuler/build_x86_systemd_mcs/UniProton/demos/x86_64/include/libc/include -I /home/openeuler/build_x86_systemd_mcs/UniProton/demos/x86_64/include" CXX=/usr1/openeuler/gcc/openeuler_gcc_x86_64/bin/x86_64-openeuler-linux-gnu-g++ --host x86
make
make install
popd

cp ./modbus/lib/*.a ../libs