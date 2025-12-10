// modified from limine/test/limine (Mintsuki, BSD)

#define REQUEST __attribute__((section(".limine_requests")))

__attribute__((cold)) void hcf(void)
{
	__asm__("cli");
	for (;;) {
		__asm__("hlt");
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

#include "../baremetal/memory.c"
#include "../flanterm/src/flanterm.c"
#include "../flanterm/src/flanterm_backends/fb.c"

#include "types.c"
#include "bytearray.c"
#include "panic.c"
#include "../baremetal/framebuffer.c"

extern void effektMain();

void kmain(void);
void kmain(void)
{
	fb_init();
	memory_init();

	effektMain();
	hcf();
}
