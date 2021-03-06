/*
 * Copyright 2012 Calxeda, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _ASM_ARM_PERCPU_H_
#define _ASM_ARM_PERCPU_H_

/*
 * Same as asm-generic/percpu.h, except that we store the per cpu offset
 * in the TPIDRPRW. TPIDRPRW only exists on V6K and V7
 */
#if defined(CONFIG_SMP) && !defined(CONFIG_CPU_V6)
static inline void set_my_cpu_offset(unsigned long off)
{
	/* Set TPIDRPRW */

/* IAMROOT-12A:
 * ------------
 * percpu 자료구조에서 사용되는 cpu마다 개별적으로 가지는 값.
 * 받아온 off 값을 TPIDRPRW register에 쓴다.
 * inline assembly 참고: https://wiki.kldp.org/KoreanDoc/html/EmbeddedKernel-KLDP/app3.basic.html
 * 질문: 왜 메모리에 할당하지 않고 CP15 register를 이용하는지?
 *          메모리를 사용한 offset은 메모리 접근이 2번 필요한데
 *          속도 향상을 위해서 사용하지 않는 register를 사용함.
 *          (이전에는 메모리에 할당했었다)
 *          참고: https://lwn.net/Articles/527927/
 *
 * The asm "memory" constraints are needed here to ensure the percpu offset
 * gets reloaded.
 */
	asm volatile("mcr p15, 0, %0, c13, c0, 4" : : "r" (off) : "memory");
}

/* IAMROOT-12AB:
 * -------------
 * 사용하지 않는 TPIDRPRW를 사용하여 per-cpu 데이터의 base offset를 읽어낸다.
 * 
 * 기존에는 메모리 변수를 사용했었는데 per-cpu의 속도를 향상시키기 위해서
 * 메모리를 사용하지 않고 사용하지 않는 레지스터를 이용하여 연산에 사용한다.
 */
static inline unsigned long __my_cpu_offset(void)
{
	unsigned long off;

	/*
	 * Read TPIDRPRW.
	 * We want to allow caching the value, so avoid using volatile and
	 * instead use a fake stack read to hazard against barrier().
	 */
	asm("mrc p15, 0, %0, c13, c0, 4" : "=r" (off)
		: "Q" (*(const unsigned long *)current_stack_pointer));

	return off;
}
#define __my_cpu_offset __my_cpu_offset()
#else
#define set_my_cpu_offset(x)	do {} while(0)

#endif /* CONFIG_SMP */

#include <asm-generic/percpu.h>

#endif /* _ASM_ARM_PERCPU_H_ */
