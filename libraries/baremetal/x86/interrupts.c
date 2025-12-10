#ifndef EFFEKT_INTERRUPTS_C
#define EFFEKT_INTERRUPTS_C

#include <stdint.h>

#define INT_GATE 0x8e
#define INT_TRAP 0xef
#define INT_USER 0x60

struct int_frame {
	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags;
} PACKED;

struct int_frame_user {
	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags;
	uint32_t useresp, ss;
} PACKED;

struct idt_entry {
	uint16_t base_low;
	uint16_t sel; // Kernel segment
	uint8_t ist;
	uint8_t flags;
	uint16_t base_mid;
	uint32_t base_high;
	uint32_t zero;
} PACKED;

struct idt_ptr {
	uint16_t size;
	void *base;
} PACKED;

__attribute__((aligned(0x10))) static struct idt_entry idt[256] = { 0 };
static struct idt_ptr idt_ptr = { 0 };

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags)
{
	struct idt_entry *descriptor = &idt[vector];

	descriptor->base_low = (uint64_t)isr & 0xFFFF;
	descriptor->sel = 40; // GDT_OFFSET_KERNEL_CODE;
	descriptor->ist = 0;
	descriptor->flags = flags;
	descriptor->base_mid = ((uint64_t)isr >> 16) & 0xFFFF;
	descriptor->base_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
	descriptor->zero = 0;
}

extern void *isr_stub_table[];

static struct Pos effekt_interrupt_handler;

void c_idt_init(struct Pos);
void c_idt_init(struct Pos callback)
{
	effekt_interrupt_handler = callback;

	idt_ptr.base = &idt;
	idt_ptr.size = (uint16_t)sizeof(idt) - 1;

	for (uint8_t vector = 0; vector < 32; vector++)
		idt_set_descriptor(vector, isr_stub_table[vector], INT_GATE);

	__asm__ volatile("lidt %0" : : "m"(idt_ptr));
	__asm__ volatile("sti");
}

void *interrupt_handler(struct int_frame *frame);
void *interrupt_handler(struct int_frame *frame)
{
	run_Int(effekt_interrupt_handler, frame->int_no);
	return frame;
}

#endif
