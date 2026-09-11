QEMU=qemu-system-x86_64
LIBRARY := lib/common/
EFFEKTFLAGS := --build --backend llvm --optimize --baremetal -l $(LIBRARY) --clang-includes limine.h --includes src/ lib/baremetal/ lib/shared/ usr/
RESOURCES := $(wildcard usr/res/*)

all: image.hdd

lib/limine.h:
	curl -Lo $@ https://codeberg.org/Limine/limine-protocol/raw/branch/trunk/include/limine.h

out/interrupts.o: lib/baremetal/x86/interrupts.asm
	nasm -f elf64 $< -o $@

out/fpu.o: lib/baremetal/x86/fpu.asm
	nasm -f elf64 $< -o $@

out/main.o: lib/limine.h
	effekt src/main.effekt $(EFFEKTFLAGS)

out/limine.conf: limine.conf $(RESOURCES)
	mkdir -p out
	cp $< $@
	for res in $(RESOURCES); do \
		echo "    module_path: boot():/res/$$(basename $$res)" >> $@; \
	done

out/effektos: out/main.o out/interrupts.o out/fpu.o
	ld -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 --gc-sections -T linker.lds -o $@ out/main.o out/main.ll.o out/interrupts.o out/fpu.o

ovmf/OVMF.fd:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && unzip OVMF-X64.zip

image.iso: out/effektos out/limine.conf
	make -C limine
	mkdir -p iso_root
	mkdir -p iso_root/boot
	cp -v out/effektos iso_root/boot/effektos
	mkdir -p iso_root/boot/limine
	cp -v out/limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/res
	$(if $(RESOURCES),cp -v $(RESOURCES) iso_root/res/)
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label iso_root -o image.iso
	./limine/limine bios-install image.iso

image.hdd: out/effektos out/limine.conf
	rm -f image.hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=image.hdd
	PATH=$$PATH:/usr/sbin:/sbin sgdisk image.hdd -n 1:2048 -t 1:ef00 -m 1
	make -C limine
	./limine/limine bios-install image.hdd
	mformat -i image.hdd@@1M
	mmd -i image.hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine ::/res
	mcopy -i image.hdd@@1M out/effektos ::/boot/effektos
	mcopy -i image.hdd@@1M out/limine.conf limine/limine-bios.sys ::/boot/limine
	$(if $(RESOURCES),mcopy -i image.hdd@@1M $(RESOURCES) ::/res)
	mcopy -i image.hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i image.hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

qemu-disk: image.hdd
	$(QEMU) -accel kvm -M q35 -m 2G -hda image.hdd -serial stdio -vga std

qemu-disk-debug: image.hdd
	$(QEMU) -accel kvm -M q35 -m 2G -hda image.hdd -serial stdio -vga std -no-reboot -d guest_errors,unimp,pcall,int,exec -D qemu.log

qemu-iso: image.iso
	$(QEMU) -accel kvm -M q35 -m 2G -cdrom image.iso -boot d -serial stdio

qemu-iso-uefi: image.iso ovmf/OVMF.fd
	$(QEMU) -accel kvm -M q35 -m 2G -bios ovmf/OVMF.fd -cdrom image.iso -boot d
