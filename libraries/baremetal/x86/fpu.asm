bits 64

global enable_sse

section .text

enable_sse:
	; CR0:
	; clear EM (bit 2)
	; clear TS (bit 3)
	; set   MP (bit 1)

	mov rax, cr0
	and rax, ~(1 << 2)
	and rax, ~(1 << 3)
	or  rax,  (1 << 1)
	mov cr0, rax

	; CR4:
	; set OSFXSR	  (bit 9)
	; set OSXMMEXCPT  (bit 10)

	mov rax, cr4
	or  rax, (1 << 9)
	or  rax, (1 << 10)
	mov cr4, rax

	fninit

	ret
