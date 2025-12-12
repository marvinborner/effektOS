// based on SHMALL
// Copyright (c) 2017, Chris Careaga (MIT)
// Copyright (c) 2025, Marvin Borner (MIT)

#include <stdint.h>
#include <stddef.h>

__attribute__((noreturn)) void hole(const char *message);

#define ALIGN_BTYES sizeof(long)

#define HEAP_INIT_SIZE 0x1000000
#define HEAP_MAX_SIZE 0xF000000
#define HEAP_MIN_SIZE 0x1000000

#define MIN_ALLOC_SZ 4

#define MIN_WILDERNESS 0x2000
#define MAX_WILDERNESS 0x1000000

#define BIN_COUNT 9
#define BIN_MAX_IDX (BIN_COUNT - 1)

typedef struct node_t {
	unsigned int hole;
	unsigned int size;
	struct node_t *next;
	struct node_t *prev;
} node_t;

typedef struct {
	node_t *header;
} footer_t;

typedef struct {
	node_t *head;
} bin_t;

typedef struct {
	long start;
	long end;
	bin_t *bins[BIN_COUNT];
} heap_t;

void heap_init(heap_t *heap, long start);

void *heap_alloc(heap_t *heap, size_t size);
void heap_free(heap_t *heap, void *p);
unsigned int expand(heap_t *heap, size_t sz);
void contract(heap_t *heap, size_t sz);
unsigned int get_bin_index(size_t sz);
void create_foot(node_t *head);
footer_t *get_foot(node_t *head);
node_t *get_wilderness(heap_t *heap);

static unsigned int overhead = sizeof(footer_t) + sizeof(node_t);

static void add_node(bin_t *bin, node_t *node)
{
	node->next = 0;
	node->prev = 0;

	if (!bin->head) {
		bin->head = node;
		return;
	}

	node_t *current = bin->head;
	node_t *previous = 0;
	while (current && current->size <= node->size) {
		previous = current;
		current = current->next;
	}

	if (!current) {
		previous->next = node;
		node->prev = previous;
	} else {
		if (previous) {
			node->next = current;
			previous->next = node;

			node->prev = previous;
			current->prev = node;
		} else {
			node->next = bin->head;
			bin->head->prev = node;
			bin->head = node;
		}
	}
}

static void remove_node(bin_t *bin, node_t *node)
{
	if (!bin->head)
		return;
	if (bin->head == node) {
		bin->head = bin->head->next;
		return;
	}

	node_t *temp = bin->head->next;
	while (temp) {
		if (temp == node) {
			if (!temp->next) {
				temp->prev->next = 0;
			} else {
				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
			}
			return;
		}
		temp = temp->next;
	}
}

static node_t *get_best_fit(bin_t *bin, size_t size)
{
	if (!bin->head)
		return 0;

	node_t *temp = bin->head;

	while (temp) {
		if (temp->size >= size)
			return temp;
		temp = temp->next;
	}
	return 0;
}

unsigned int offset = 8;

void heap_init(heap_t *heap, long start)
{
	node_t *init_region = (node_t *)start;
	init_region->hole = 1;
	init_region->size =
		(HEAP_INIT_SIZE) - sizeof(node_t) - sizeof(footer_t);

	create_foot(init_region);

	add_node(heap->bins[get_bin_index(init_region->size)], init_region);

	heap->start = start;
	heap->end = start + HEAP_INIT_SIZE;
}

void *heap_alloc(heap_t *heap, size_t size)
{
	size = ((size + ALIGN_BTYES - 1) / ALIGN_BTYES) * ALIGN_BTYES;

	unsigned int index = get_bin_index(size);
	bin_t *temp = (bin_t *)heap->bins[index];
	node_t *found = get_best_fit(temp, size);

	while (!found) {
		if (index + 1 >= BIN_COUNT)
			return NULL;

		temp = heap->bins[++index];
		found = get_best_fit(temp, size);
	}

	if ((found->size - size) > (overhead + MIN_ALLOC_SZ)) {
		node_t *split = (node_t *)(((char *)found + sizeof(node_t) +
					    sizeof(footer_t)) +
					   size);
		split->size =
			found->size - size - sizeof(node_t) - sizeof(footer_t);
		split->hole = 1;

		create_foot(split);

		unsigned int new_idx = get_bin_index(split->size);

		add_node(heap->bins[new_idx], split);

		found->size = size;
		create_foot(found);
	}

	found->hole = 0;
	remove_node(heap->bins[index], found);

	node_t *wild = get_wilderness(heap);
	if (wild->size < MIN_WILDERNESS) {
		unsigned int success = expand(heap, 0x1000);
		if (success == 0) {
			return NULL;
		}
	} else if (wild->size > MAX_WILDERNESS) {
		contract(heap, 0x1000);
	}

	found->prev = 0;
	found->next = 0;
	return &found->next;
}

void heap_free(heap_t *heap, void *p)
{
	bin_t *list;
	footer_t *new_foot, *old_foot;

	node_t *head = (node_t *)((char *)p - offset);
	if (head == (node_t *)(uintptr_t)heap->start) {
		head->hole = 1;
		add_node(heap->bins[get_bin_index(head->size)], head);
		return;
	}

	node_t *next = (node_t *)((char *)get_foot(head) + sizeof(footer_t));
	footer_t *f = (footer_t *)((char *)head - sizeof(footer_t));
	node_t *prev = f->header;

	if (prev->hole) {
		list = heap->bins[get_bin_index(prev->size)];
		remove_node(list, prev);

		prev->size += overhead + head->size;
		new_foot = get_foot(head);
		new_foot->header = prev;

		head = prev;
	}

	if (next->hole) {
		list = heap->bins[get_bin_index(next->size)];
		remove_node(list, next);

		head->size += overhead + next->size;

		old_foot = get_foot(next);
		old_foot->header = 0;
		next->size = 0;
		next->hole = 0;

		new_foot = get_foot(head);
		new_foot->header = head;
	}

	head->hole = 1;
	add_node(heap->bins[get_bin_index(head->size)], head);
}

void *memcpy(void *dest, const void *src, size_t n);
void *heap_realloc(heap_t *heap, void *p, size_t sz)
{
	if (p == NULL)
		return heap_alloc(heap, sz);
	if (sz == 0) {
		heap_free(heap, p);
		return NULL;
	}

	node_t *head = (node_t *)((char *)p - offset);
	size_t old_size = head->size;

	if (sz <= old_size) {
		// TODO: implement shrinking
		return p;
	}

	void *new = heap_alloc(heap, sz);
	if (new == NULL)
		return NULL;
	memcpy(new, p, old_size);
	heap_free(heap, p);
	return new;
}

unsigned int expand(heap_t *heap, size_t sz)
{
	hole("expanding heap not implemented");
	(void)heap;
	(void)sz;
	return 0;
}

void contract(heap_t *heap, size_t sz)
{
	(void)heap;
	(void)sz;
	return;
}

unsigned int get_bin_index(size_t sz)
{
	unsigned int index = 0;
	sz = sz < 4 ? 4 : sz;

	while (sz >>= 1)
		index++;
	index -= 2;

	if (index > BIN_MAX_IDX)
		index = BIN_MAX_IDX;
	return index;
}

void create_foot(node_t *head)
{
	footer_t *foot = get_foot(head);
	foot->header = head;
}

footer_t *get_foot(node_t *node)
{
	return (footer_t *)((char *)node + sizeof(node_t) + node->size);
}

node_t *get_wilderness(heap_t *heap)
{
	footer_t *wild_foot =
		(footer_t *)((char *)heap->end - sizeof(footer_t));
	return wild_foot->header;
}