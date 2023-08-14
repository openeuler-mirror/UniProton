AS = /opt/buildtools/hcc_x86/bin/x86_64-target-linux-gnu-as
CC = gcc
AR = /opt/buildtools/hcc_x86/bin/x86_64-target-linux-gnu-ar
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump

AP_BOOT = boot

ap_boot:
	$(CC) -g -o $(AP_BOOT).o -c $(AP_BOOT).S; \
	$(LD) -N -e ap_boot_16 -Ttext=0x0 -o $(AP_BOOT).elf $(AP_BOOT).o; \
	$(OBJDUMP) -D -S $(AP_BOOT).elf > $(AP_BOOT).asm
	$(OBJCOPY) -S -O binary $(AP_BOOT).elf ap_boot;

clean:
	rm -rf *.o *.asm ap_boot