// modified from limine/test/limine (Mintsuki, BSD)

static struct flanterm_context *ft_ctx = 0;

#include "memory.c"
#include "../limine.h"
#include "../flanterm/src/flanterm.c"
#include "../flanterm/src/flanterm_backends/fb.c"
#include "e9print.c"

#include "types.c"
#include "bytearray.c"
#include "io.c"
#include "panic.c"

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

void kmain(void);
void kmain(void)
{
	e9_init();
	e9_printf("hello, world");

	struct limine_framebuffer *fb =
		framebuffer_request.response->framebuffers[0];

	ft_ctx = flanterm_fb_init(NULL, NULL, fb->address, fb->width,
				  fb->height, fb->pitch, fb->red_mask_size,
				  fb->red_mask_shift, fb->green_mask_size,
				  fb->green_mask_shift, fb->blue_mask_size,
				  fb->blue_mask_shift, NULL, NULL, NULL, NULL,
				  NULL, NULL, NULL, NULL, 0, 0, 1, 0, 0, 0, 0);

	if (memmap_request.response == NULL) {
		e9_printf("Memory map not passed");
		hcf();
	}

	size_t largest_memory_size = 0;
	void *largest_memory_area = 0;

	struct limine_memmap_response *memmap_response =
		memmap_request.response;
	for (size_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry *e = memmap_response->entries[i];
		if (e->type == LIMINE_MEMMAP_USABLE &&
		    e->length > largest_memory_size) {
			largest_memory_size = e->length;
			largest_memory_area = (void *)e->base;
		}
	}

	memory_init(largest_memory_area);

	effektMain();
	e9_print("Effekt returned, hcfing!\n");
	hcf();
}
