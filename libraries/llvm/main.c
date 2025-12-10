static struct flanterm_context *ft_ctx = 0;

#include "memory.c"
#include "../limine.h"
#include "../flanterm/src/flanterm.c"
#include "../flanterm/src/flanterm_backends/fb.c"
#include "e9print.c"

#define REQUEST __attribute__((section(".limine_requests")))

extern void effektMain();

REQUEST static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0,
	.response = NULL
};

REQUEST static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0,
	.response = NULL
};

static void hcf(void)
{
	__asm__("cli");
	for (;;) {
		__asm__("hlt");
	}
}

void kmain(void);
void kmain(void)
{
	e9_init();
	e9_print("hello, world\n");

	struct limine_framebuffer *fb =
		framebuffer_request.response->framebuffers[0];

	ft_ctx = flanterm_fb_init(NULL, NULL, fb->address, fb->width,
				  fb->height, fb->pitch, fb->red_mask_size,
				  fb->red_mask_shift, fb->green_mask_size,
				  fb->green_mask_shift, fb->blue_mask_size,
				  fb->blue_mask_shift, NULL, NULL, NULL, NULL,
				  NULL, NULL, NULL, NULL, 0, 0, 1, 0, 0, 0, 0);

	e9_print("Framebuffer initialized!\n");
	hcf();
}
