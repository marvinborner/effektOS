#ifndef EFFEKT_PANIC_C
#define EFFEKT_PANIC_C

// TODO:
// this should _morally_ be using `stderr`, but we don't tee it in tests
// see PR #823 & issue #815 for context

__attribute__((cold)) void hole(const char *message)
{
	/* e9_printf("PANIC: %s not implemented yet\n", message); */
	hcf();
}

__attribute__((cold)) void duplicated_prompt()
{
	/* e9_printf("PANIC: Continuation invoked itself\n"); */
	hcf();
}

#endif
