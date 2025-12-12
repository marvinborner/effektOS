#ifndef EFFEKT_FB_C
#define EFFEKT_FB_C

void fb_init(void)
{
	struct limine_framebuffer *fb =
		framebuffer_request.response->framebuffers[0];

	ft_ctx = flanterm_fb_init(NULL, NULL, fb->address, fb->width,
				  fb->height, fb->pitch, fb->red_mask_size,
				  fb->red_mask_shift, fb->green_mask_size,
				  fb->green_mask_shift, fb->blue_mask_size,
				  fb->blue_mask_shift, NULL, NULL, NULL, NULL,
				  NULL, NULL, NULL, NULL, 0, 0, 1, 0, 0, 0, 0);
}

void fb_putchar(char ch)
{
	if (ft_ctx != NULL) {
		flanterm_write(ft_ctx, &ch, 1);
	}
}

void fb_print(const char *s)
{
	for (const char *p = s; *p; p++)
		fb_putchar(*p);
}

void c_fb_print(String text)
{
	for (uint64_t j = 0; j < text.tag; ++j) {
		fb_putchar(c_bytearray_data(text)[j]);
	};
	erasePositive(text);
}

#endif
