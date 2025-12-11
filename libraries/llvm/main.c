// modified from limine/test/limine (Mintsuki, BSD)

#define REQUEST __attribute__((section(".limine_requests")))
#define PACKED __attribute__((packed))

__attribute__((noreturn)) void hcf(void)
{
	__asm__ volatile("cli");
	for (;;) {
		__asm__ volatile("hlt");
	}
}

static struct flanterm_context *ft_ctx = 0;

#include "../limine.h"

REQUEST static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0,
	.response = 0
};

REQUEST static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0,
	.response = 0
};

#include "../baremetal/heap.c"
#include "../baremetal/memory.c"
#include "../flanterm/src/flanterm.c"
#include "../flanterm/src/flanterm_backends/fb.c"

#include "types.c"
#include "bytearray.c"
#include "../baremetal/framebuffer.c"
#include "panic.c"

#include "../baremetal/x86/interrupts.c"

extern void effektMain(void);

void kmain(void);
void kmain(void)
{
	// gdt_init();
	memory_init();
	fb_init();

	effektMain();
	fb_print("Effekt returned?!");
	while(1)
		__asm__ volatile("hlt");
}
