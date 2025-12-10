// modified from limine/test/memory (Mintsuki, BSD)

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *pdest = dest;
	const uint8_t *psrc = src;

	for (size_t i = 0; i < n; i++) {
		pdest[i] = psrc[i];
	}

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t)c;
	}

	return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
	uint8_t *pdest = dest;
	const uint8_t *psrc = src;

	if (src > dest) {
		for (size_t i = 0; i < n; i++) {
			pdest[i] = psrc[i];
		}
	} else if (src < dest) {
		for (size_t i = n; i > 0; i--) {
			pdest[i - 1] = psrc[i - 1];
		}
	}

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *p1 = s1;
	const uint8_t *p2 = s2;

	for (size_t i = 0; i < n; i++) {
		if (p1[i] != p2[i])
			return p1[i] < p2[i] ? -1 : 1;
	}

	return 0;
}

static void *kheap = 0;
void memory_init(void *heap)
{
	kheap = heap;
}

void *malloc(size_t size)
{
	void *ptr = kheap;
	kheap = (uint8_t *)kheap + size;
	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	void *new = malloc(size);
	memcpy(new, ptr, size); // might leak but idc
	return new;
}

void free(void *ptr)
{
	return;
}
