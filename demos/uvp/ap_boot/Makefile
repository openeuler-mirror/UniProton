GCC_FILE=/opt/buildtools/openeuler_gcc_x86_64/bin/x86_64-openeuler-linux-gnu-gcc
ifneq ($(GCC_FILE), $(wildcard $(GCC_FILE)))
	TOOLCHAIN_PATH=/usr1/openeuler/gcc/openeuler_gcc_x86_64
else
	TOOLCHAIN_PATH=/opt/buildtools/openeuler_gcc_x86_64
endif

AS = $(TOOLCHAIN_PATH)/bin/x86_64-openeuler-linux-gnu-as
CC = $(TOOLCHAIN_PATH)/bin/x86_64-openeuler-linux-gnu-gcc
AR = $(TOOLCHAIN_PATH)/bin/x86_64-openeuler-linux-gnu-ar
LD = $(TOOLCHAIN_PATH)/bin/x86_64-openeuler-linux-gnu-ld
OBJCOPY = $(TOOLCHAIN_PATH)/bin/x86_64-openeuler-linux-gnu-objcopy
OBJDUMP = $(TOOLCHAIN_PATH)/bin/x86_64-openeuler-linux-gnu-objdump

AP_BOOT = boot
ap_boot:
	$(CC) -g -o $(AP_BOOT).o -c $(AP_BOOT).S; \
	$(LD) -N -e ap_boot_16 -Ttext=0x0 -o $(AP_BOOT).elf $(AP_BOOT).o; \
	$(OBJDUMP) -S $(AP_BOOT).elf > $(AP_BOOT).asm
	$(OBJCOPY) -O binary -j .text $(AP_BOOT).elf ap_boot;

clean:
	rm -rf *.o *.asm *.elf ap_boot
