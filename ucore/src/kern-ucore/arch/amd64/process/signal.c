#include <arch.h>
#include <mp.h>
#include <vmm.h>
#include <proc.h>
#include <slab.h>
#include <error.h>
#include "signal.h"

int __sig_setup_frame(int sign, struct sigaction *act, sigset_t oldset,
		      struct trapframe *tf) {
        struct mm_struct *mm = current->mm;
        uintptr_t stack = current->signal_info.sas_ss_sp;
        if (stack == 0) {
                stack = tf->tf_rsp;
        }
        
        struct sigframe *kframe = kmalloc(sizeof(struct sigframe));
	if (!kframe)
		return -E_NO_MEM;
	memset(kframe, 0, sizeof(struct sigframe));
        
        kframe->sign = sign;
        kframe->tf = *tf;
        kframe->old_blocked = oldset;

        // this magic number is code of "movl $149, %eax    int $0x80"
        // by setting the top of user stack a pointer to this,
        // the handler will call sigreturn syscall when is returns
        kframe->retcode = 0x0080cd00000095b8ull;

        struct sigframe *frame =
	    (struct sigframe *)((stack - sizeof(struct sigframe)) & 0xfffffffffffffff8ull);
	lock_mm(mm);
	{
		if (!copy_to_user(mm, frame, kframe, sizeof(struct sigframe))) {
			unlock_mm(mm);
			kfree(kframe);
			return -E_INVAL;
		}
	}
	unlock_mm(mm);
	kfree(kframe);

        frame->pretcode = (uintptr_t)&(frame->retcode);
        

        tf->tf_rsp = (uintptr_t)frame;
        tf->tf_rip = (uintptr_t)act->sa_handler;
        tf->tf_regs.reg_rdi = (uint64_t)sign;
        tf->tf_regs.reg_rcx = tf->tf_regs.reg_rdx = 0;

        //we don't need to assign cs ds ss es...
        return 0;
}

// do syscall sigreturn, reset the user stack and eip
int do_sigreturn() {
	struct mm_struct *mm = current->mm;
	if (!current)
		return -E_INVAL;
	struct sighand_struct *sighand = current->signal_info.sighand;
	if (!sighand)
		return -E_INVAL;

	struct sigframe *kframe = kmalloc(sizeof(struct sigframe));
	if (!kframe)
		return -E_NO_MEM;

	struct sigframe *frame =
	    (struct sigframe *)(current->tf->tf_rsp - sizeof(uintptr_t));
	lock_mm(mm);
	{
		if (!copy_from_user
		    (mm, kframe, frame, sizeof(struct sigframe), 0)) {
			unlock_mm(mm);
			kfree(kframe);
			return -E_INVAL;
		}
	}
	unlock_mm(mm);
	/* check the trapframe */
	if (trap_in_kernel(&kframe->tf)) {
		do_exit(-E_KILLED);
		return -E_INVAL;
	}

	lock_sig(sighand);
	current->signal_info.blocked = kframe->old_blocked;
	sig_recalc_pending(current);
	unlock_sig(sighand);

	*(current->tf) = kframe->tf;
	kfree(kframe);
	return 0;
}

