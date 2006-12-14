/*
 * Copyright (C) 2005 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup sparc64
 * @{
 */
/** @file
 */

#include <arch.h>
#include <debug.h>
#include <config.h>
#include <arch/trap/trap.h>
#include <arch/console.h>
#include <proc/thread.h>
#include <console/console.h>
#include <arch/boot/boot.h>
#include <arch/arch.h>
#include <arch/asm.h>
#include <arch/mm/page.h>
#include <arch/stack.h>
#include <genarch/ofw/ofw_tree.h>
#include <userspace.h>
#include <ddi/irq.h>

bootinfo_t bootinfo;

void arch_pre_main(void)
{
	/* Copy init task info. */
	init.cnt = bootinfo.taskmap.count;
	
	uint32_t i;

	for (i = 0; i < bootinfo.taskmap.count; i++) {
		init.tasks[i].addr = (uintptr_t) bootinfo.taskmap.tasks[i].addr;
		init.tasks[i].size = bootinfo.taskmap.tasks[i].size;
	}
	
	/* Copy boot allocations info. */
	ballocs.base = bootinfo.ballocs.base;
	ballocs.size = bootinfo.ballocs.size;
	
	ofw_tree_init(bootinfo.ofw_root);
}

void arch_pre_mm_init(void)
{
	if (config.cpu_active == 1)
		trap_init();
}

void arch_post_mm_init(void)
{
	if (config.cpu_active == 1) {
		irq_init(1<<11, 128);
		standalone_sparc64_console_init();
	}
}

void arch_post_cpu_init(void)
{
}

void arch_pre_smp_init(void)
{
}

void arch_post_smp_init(void)
{
	thread_t *t;

	if (config.cpu_active == 1) {
		/*
	         * Create thread that polls keyboard.
	         */
		t = thread_create(kkbdpoll, NULL, TASK, 0, "kkbdpoll", true);
		if (!t)
			panic("cannot create kkbdpoll\n");
		thread_ready(t);
	}
}

/** Calibrate delay loop.
 *
 * On sparc64, we implement delay() by waiting for the TICK register to
 * reach a pre-computed value, as opposed to performing some pre-computed
 * amount of instructions of known duration. We set the delay_loop_const
 * to 1 in order to neutralize the multiplication done by delay().
 */
void calibrate_delay_loop(void)
{
	CPU->delay_loop_const = 1;
}

/** Wait several microseconds.
 *
 * We assume that interrupts are already disabled.
 *
 * @param t Microseconds to wait.
 */
void asm_delay_loop(const uint32_t usec)
{
	uint64_t stop = tick_read() + (uint64_t) usec * (uint64_t)
		CPU->arch.clock_frequency / 1000000;

	while (tick_read() < stop)
		;
}

/** Switch to userspace. */
void userspace(uspace_arg_t *kernel_uarg)
{
	switch_to_userspace((uintptr_t) kernel_uarg->uspace_entry,
		((uintptr_t) kernel_uarg->uspace_stack) + STACK_SIZE
		- (ALIGN_UP(STACK_ITEM_SIZE, STACK_ALIGNMENT) + STACK_BIAS),
		(uintptr_t) kernel_uarg->uspace_uarg);

	for (;;)
		;
	/* not reached */
}

/** @}
 */
