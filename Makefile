QEMU=qemu-system-x86_64
LIBRARY := libraries/common/
EFFEKTFLAGS := --build --backend llvm --no-optimize --baremetal -l $(LIBRARY) --clang-includes limine.h --includes libraries/baremetal/

all: image.hdd

libraries/limine.h:
	curl -Lo $@ https://codeberg.org/Limine/limine-protocol/raw/branch/trunk/include/limine.h

out/interrupts.o: libraries/baremetal/x86/interrupts.asm
	nasm -f elf64 libraries/baremetal/x86/interrupts.asm -o out/interrupts.o

out/main.o: libraries/limine.h
	effekt src/main.effekt $(EFFEKTFLAGS)

out/effektos: out/main.o out/interrupts.o
	ld -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 --gc-sections -T linker.lds -o $@ out/main.o out/main.ll.o out/interrupts.o

ovmf/OVMF.fd:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && unzip OVMF-X64.zip

image.iso: out/effektos
	make -C limine
	mkdir -p iso_root
	mkdir -p iso_root/boot
	cp -v out/effektos iso_root/boot/effektos
	mkdir -p iso_root/boot/limine
	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label iso_root -o image.iso
	./limine/limine bios-install image.iso

image.hdd: out/effektos
	rm -f image.hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=image.hdd
	PATH=$$PATH:/usr/sbin:/sbin sgdisk image.hdd -n 1:2048 -t 1:ef00 -m 1
	make -C limine
	./limine/limine bios-install image.hdd
	mformat -i image.hdd@@1M
	mmd -i image.hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i image.hdd@@1M out/effektos ::/boot/effektos
	mcopy -i image.hdd@@1M limine.conf limine/limine-bios.sys ::/boot/limine
	mcopy -i image.hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i image.hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

qemu-disk: image.hdd
	$(QEMU) -M q35 -m 2G -hda image.hdd -serial stdio -vga std

qemu-disk-debug: image.hdd
	$(QEMU) -M q35 -m 2G -hda image.hdd -serial stdio -vga std -no-reboot -d guest_errors,unimp,pcall,int,exec -D qemu.log

qemu-iso: image.iso
	$(QEMU) -M q35 -m 2G -cdrom image.iso -boot d -serial stdio

qemu-iso-uefi: image.iso ovmf/OVMF.fd
	$(QEMU) -M q35 -m 2G -bios ovmf/OVMF.fd -cdrom image.iso -boot d
