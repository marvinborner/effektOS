#ifndef EFFEKT_INTERRUPTS_C
#define EFFEKT_INTERRUPTS_C

#include <stdint.h>

#define INT_GATE 0x8e
#define INT_TRAP 0xef
#define INT_USER 0x60

struct int_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code, rip, cs, rflags, rsp, ss;
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

extern void *int_table[];

static struct Pos effekt_interrupt_handler;

void c_idt_install_handler(struct Pos callback) {
	effekt_interrupt_handler = callback;
}

void c_idt_init(struct Pos);
void c_idt_init(struct Pos callback)
{
	c_idt_install_handler(callback);

	idt_ptr.base = &idt;
	idt_ptr.size = (uint16_t)sizeof(idt) - 1;

	for (int i = 0; i < 256; i++) {
		idt_set_descriptor(i, int_table[i], INT_GATE);
	}

	__asm__ volatile("lidt %0" : : "m"(idt_ptr));
	__asm__ volatile("sti");
}

void irq_handler(struct int_frame *);
void irq_handler(struct int_frame *frame)
{
	run_Int(effekt_interrupt_handler, frame->int_no);
	fb_print("run done");
}

void except_handler(struct int_frame *);
void except_handler(struct int_frame *frame)
{
	run_Int(effekt_interrupt_handler, frame->int_no);
	fb_print("run done");
}

#endif
