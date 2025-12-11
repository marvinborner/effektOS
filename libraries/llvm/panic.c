#ifndef EFFEKT_PANIC_C
#define EFFEKT_PANIC_C

// TODO:
// this should _morally_ be using `stderr`, but we don't tee it in tests
// see PR #823 & issue #815 for context

__attribute__((cold)) void hole(const char *message)
{
	(void)message;
	fb_print("PANIC: not implemented yet\n");
	hcf();
}

__attribute__((cold)) void duplicated_prompt()
{
	fb_print("PANIC: Continuation invoked itself\n");
	hcf();
}

#endif
