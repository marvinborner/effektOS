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

// --- allocation ---

static void *kheap = 0;
static heap_t *uheap = 0;

void *kmalloc(size_t size)
{
	void *ptr = kheap;
	kheap = (uint8_t *)kheap + size;
	return ptr;
}

void memory_init(void)
{
	if (memmap_request.response == NULL)
		hcf();

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

	kheap = largest_memory_area;
	uheap = kmalloc(sizeof(*uheap));
	memset(uheap, 0, sizeof(*uheap));

	void *region = kmalloc(HEAP_INIT_SIZE);
	memset(region, 0, HEAP_INIT_SIZE);

	for (int i = 0; i < BIN_COUNT; i++) {
		uheap->bins[i] = kmalloc(sizeof(bin_t));
		memset(uheap->bins[i], 0, sizeof(bin_t));
	}

	heap_init(uheap, (long)region);
}

void *malloc(size_t size)
{
	return heap_alloc(uheap, size);
}

void *realloc(void *p, size_t size)
{
	return heap_realloc(uheap, p, size);
}

void *calloc(size_t n, size_t size)
{
	// TODO: handle integer overflow (return 0)
	void *p = heap_alloc(uheap, n * size);
	memset(p, 0, n * size);
	return p;
}

void free(void *ptr)
{
	heap_free(uheap, ptr);
}