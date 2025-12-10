#ifndef EFFEKT_IO_C
#define EFFEKT_IO_C

void c_io_println(String text)
{
	for (uint64_t j = 0; j < text.tag; ++j) {
		e9_putc(c_bytearray_data(text)[j]);
	};
	erasePositive(text);
	e9_putc('\n');
}

#endif
