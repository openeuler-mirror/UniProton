#bash ./build.sh $APP
qemu-system-riscv64 -bios none -M virt -m 512M  -nographic -kernel out/rv64virt.elf -smp 1  
