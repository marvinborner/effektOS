#ifndef EFFEKT_PANIC_C
#define EFFEKT_PANIC_C

// TODO:
// this should _morally_ be using `stderr`, but we don't tee it in tests
// see PR #823 & issue #815 for context

__attribute__((noreturn)) void hole(const char *message)
{
	fb_print("PANIC: not implemented yet\n");
	fb_print(message);
	hcf();
}

__attribute__((noreturn)) void duplicated_prompt(void)
{
	fb_print("PANIC: Continuation invoked itself\n");
	hcf();
}

#endif
