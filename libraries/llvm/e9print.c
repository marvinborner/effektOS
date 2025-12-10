// modified from limine/test/e9print (Mintsuki, BSD)

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define PORT 0x3f8

static const char CONVERSION_TABLE[] = "0123456789abcdef";

static void outb(unsigned short port, unsigned char data)
{
	__asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

static unsigned char inb(unsigned short port)
{
	unsigned char data;
	__asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
	return data;
}

void e9_init(void)
{
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xc7);

	// Test serial chip
	outb(PORT + 4, 0x1e); // Enable loopback
	outb(PORT + 0, 0xae); // Write

	// Activate
	outb(PORT + 4, 0x0f);
}

static int e9_empty(void)
{
	return inb(PORT + 5) & 0x20;
}

void e9_putc(char ch)
{
	if (ft_ctx)
		flanterm_write(ft_ctx, &ch, 1);

	while (e9_empty() == 0)
		;
	outb(PORT, (unsigned char)ch);
}

void e9_print(const char *msg)
{
	for (size_t i = 0; msg[i]; i++) {
		e9_putc(msg[i]);
	}
}

void e9_puts(const char *msg)
{
	e9_print(msg);
	e9_putc('\n');
}

static void e9_printhex(size_t num)
{
	int i;
	char buf[17];

	if (!num) {
		e9_print("0x0");
		return;
	}

	buf[16] = 0;

	for (i = 15; num; i--) {
		buf[i] = CONVERSION_TABLE[num % 16];
		num /= 16;
	}

	i++;
	e9_print("0x");
	e9_print(&buf[i]);
}

static void e9_printdec(size_t num)
{
	int i;
	char buf[21] = { 0 };

	if (!num) {
		e9_putc('0');
		return;
	}

	for (i = 19; num; i--) {
		buf[i] = (num % 10) + 0x30;
		num = num / 10;
	}

	i++;
	e9_print(buf + i);
}

void e9_printf(const char *format, ...)
{
	va_list argp;
	va_start(argp, format);

	while (*format != '\0') {
		if (*format == '%') {
			format++;
			if (*format == 'x') {
				e9_printhex(va_arg(argp, size_t));
			} else if (*format == 'd') {
				e9_printdec(va_arg(argp, size_t));
			} else if (*format == 's') {
				e9_print(va_arg(argp, char *));
			}
		} else {
			e9_putc(*format);
		}
		format++;
	}

	e9_putc('\n');
	va_end(argp);
}
