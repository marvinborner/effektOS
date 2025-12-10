QEMU=qemu-system-x86_64
LIBRARY := libraries/common/
EFFEKTFLAGS := --build --backend llvm --optimize --freestanding --ir-write-all -l $(LIBRARY)

all: build iso

build:
	effekt $(EFFEKTFLAGS) src/main.effekt

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && unzip OVMF-X64.zip

iso:
	make -C limine
	mkdir -p iso_root
	mkdir -p iso_root/boot
	cp -v out/main iso_root/boot/effektos
	mkdir -p iso_root/boot/limine
	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label iso_root -o image.iso
	./limine/limine bios-install image.iso

disk: build
	rm -f image.hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=image.hdd
	PATH=$$PATH:/usr/sbin:/sbin sgdisk image.hdd -n 1:2048 -t 1:ef00 -m 1
	make -C limine
	./limine/limine bios-install image.hdd
	mformat -i image.hdd@@1M
	mmd -i image.hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i image.hdd@@1M out/main ::/boot/effektos
	mcopy -i image.hdd@@1M limine.conf limine/limine-bios.sys ::/boot/limine
	mcopy -i image.hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i image.hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

qemu-disk: disk
	$(QEMU) -M q35 -m 2G -hda image.hdd

qemu-iso: iso
	$(QEMU) -M q35 -m 2G -cdrom image.iso -boot d -serial stdio

qemu-iso-uefi: iso ovmf
	$(QEMU) -M q35 -m 2G -bios ovmf/OVMF.fd -cdrom image.iso -boot d
