#ifndef __ARCH_AMD64_SIGNAL_H
#define __ARCH_AMD64_SIGNAL_H
#include <signal.h>

struct sigframe {
	uintptr_t pretcode;
	int sign;
	struct trapframe tf;
	sigset_t old_blocked;
	//there's fpstate in linux, but nothing here
	uint64_t retcode;
};

#endif
