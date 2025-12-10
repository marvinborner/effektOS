// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef EFFEKT_GDT_C
#define EFFEKT_GDT_C

#define GDT_ROOT_CODE_GATE 1
#define GDT_ROOT_DATA_GATE 2
#define GDT_USER_CODE_GATE 3
#define GDT_USER_DATA_GATE 4
#define GDT_TSS_GATE 5

#define GDT_SUPER_CODE_OFFSET (gdt_offset(GDT_ROOT_CODE_GATE))
#define GDT_SUPER_DATA_OFFSET (gdt_offset(GDT_ROOT_DATA_GATE))
#define GDT_USER_CODE_OFFSET (gdt_offset(GDT_USER_CODE_GATE) | 3)
#define GDT_USER_DATA_OFFSET (gdt_offset(GDT_USER_DATA_GATE) | 3)
#define GDT_TSS_OFFSET (gdt_offset(GDT_TSS_GATE))

#define GDT_MAX_LIMIT 0xffff
#define GDT_PRESENT (1 << 7)
#define GDT_RING3 (3 << 5)
#define GDT_DESCRIPTOR (1 << 4)
#define GDT_EXECUTABLE (1 << 3)
#define GDT_READWRITE (1 << 1)
#define GDT_ACCESSED (1 << 0)
#define GDT_GRANULARITY (0x80 | 0x00)
#define GDT_SIZE (0x40 | 0x00)
#define GDT_DATA_OFFSET 0x10

struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} PACKED;

struct gdt_ptr {
	uint16_t limit;
	void *base;
} PACKED;

struct tss_entry {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} PACKED;

static struct gdt_entry gdt[6] = { 0 };
static struct tss_entry tss = { 0 };

static struct gdt_ptr gp = { 0 };

uint8_t gdt_offset(uint8_t gate)
{
	/* assert(gate && gate < COUNT(gdt)); */
	return ((uint32_t)&gdt[gate] - (uint32_t)gdt) & 0xff;
}

static void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit,
			 uint8_t access, uint8_t gran)
{
	// Set descriptor base address
	gdt[num].base_low = (uint16_t)(base & 0xffff);
	gdt[num].base_middle = (uint8_t)((base >> 16) & 0xff);
	gdt[num].base_high = (uint8_t)((base >> 24) & 0xff);
	gdt[num].limit_low = (uint16_t)(limit & 0xffff);

	// Set granularity and access flags
	gdt[num].granularity = (uint8_t)((limit >> 16) & 0x0f);
	gdt[num].granularity |= (gran & 0xf0);
	gdt[num].access = access;
}

static void tss_write(uint32_t num, uint16_t ss0, uint32_t esp0)
{
	uint32_t base = (uint32_t)&tss;
	uint32_t limit = sizeof(tss) - 1;

	gdt_set_gate(num, base, limit,
		     GDT_PRESENT | GDT_RING3 | GDT_EXECUTABLE | GDT_ACCESSED,
		     GDT_SIZE);

	memset(&tss, 0, sizeof(tss));

	tss.ss0 = ss0;
	tss.esp0 = esp0;
	tss.cs = GDT_SUPER_CODE_OFFSET | 3;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = GDT_SUPER_DATA_OFFSET | 3;
	tss.iomap_base = sizeof(tss);
}

static void tss_flush(void)
{
	__asm__ volatile("ltr %0" ::"r"((uint16_t)(GDT_TSS_OFFSET | 3)));
}

static void gdt_flush(void)
{
	__asm__ volatile("lgdt %0" ::"m"(gp) : "memory");
}

void tss_set_stack(uint32_t esp)
{
	tss.esp0 = esp;
}

void gdt_init(void);
void gdt_init(void)
{
	// Set GDT pointer and limit
	gp.limit = sizeof(gdt) - 1;
	gp.base = gdt;

	// NULL descriptor
	gdt_set_gate(0, 0, 0, 0, 0);

	// Code segment
	gdt_set_gate(GDT_ROOT_CODE_GATE, 0, UINT32_MAX,
		     GDT_PRESENT | GDT_DESCRIPTOR | GDT_EXECUTABLE |
			     GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// Data segment
	gdt_set_gate(GDT_ROOT_DATA_GATE, 0, UINT32_MAX,
		     GDT_PRESENT | GDT_DESCRIPTOR | GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// User mode code segment
	gdt_set_gate(GDT_USER_CODE_GATE, 0, UINT32_MAX,
		     GDT_PRESENT | GDT_RING3 | GDT_DESCRIPTOR | GDT_EXECUTABLE |
			     GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// User mode data segment
	gdt_set_gate(GDT_USER_DATA_GATE, 0, UINT32_MAX,
		     GDT_PRESENT | GDT_RING3 | GDT_DESCRIPTOR | GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// Write TSS
	tss_write(GDT_TSS_GATE, GDT_SUPER_DATA_OFFSET, 0);

	// Remove old GDT and install the new changes!
	gdt_flush();
	__asm__ volatile("mov %%ax, %%ds\n"
			 "mov %%ax, %%es\n"
			 "mov %%ax, %%fs\n"
			 "mov %%ax, %%gs\n"
			 "mov %%ax, %%ss\n" ::"a"(GDT_SUPER_DATA_OFFSET)
			 : "memory");

	__asm__ volatile("pushq %0\n"
			 "leaq 1f(%%rip), %%rax\n"
			 "pushq %%rax\n"
			 "lretq\n"
			 "1:\n"
			 :
			 : "i"(0x08)
			 : "rax", "memory");

	tss_flush();
}

#endif
