global int_table
int_table:
%assign i 0
%rep 32
	dq exception%+i
	%assign i i+1
%endrep
%rep 224
	dq interrupt%+i
	%assign i i+1
%endrep

%macro pushaq 0
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8 
	push r9 
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popaq 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9 
	pop r8 
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%assign i 32
%rep 224
interrupt%+i:
	push 0
    push i
	jmp interrupt_common
%assign i i+1
%endrep

extern interrupt_handler
interrupt_common:
	pushaq 
	mov rdi, rsp
	call interrupt_handler
	popaq 
	add rsp, 16
	iretq

%assign i 0
%rep 32
%if (i < 10) || ((i > 14) && (i < 17)) || ((i > 17) && (i < 21))
exception%+i:
    push 0
    push i
	jmp exception_common
%else
exception%+i:
    push i
	jmp exception_common
%endif
%assign i i+1
%endrep

extern exception_handler
exception_common:
	pushaq 
	mov rdi, rsp
	call exception_handler
	popaq 
	add rsp, 16
	iretq