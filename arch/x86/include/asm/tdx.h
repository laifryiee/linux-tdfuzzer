/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2021-2022 Intel Corporation */
#ifndef _ASM_X86_TDX_H
#define _ASM_X86_TDX_H

#include <linux/init.h>
#include <linux/bits.h>

#include <asm/errno.h>
#include <asm/ptrace.h>
#include <asm/shared/tdx.h>

/*
 * SW-defined error codes.
 *
 * Bits 47:40 == 0xFF indicate Reserved status code class that never used by
 * TDX module.
 */
#define TDX_ERROR			_BITUL(63)
#define TDX_SW_ERROR			(TDX_ERROR | GENMASK_ULL(47, 40))
#define TDX_SEAMCALL_VMFAILINVALID	(TDX_SW_ERROR | _UL(0xFFFF0000))

#ifndef __ASSEMBLY__

/*
 * Used by the #VE exception handler to gather the #VE exception
 * info from the TDX module. This is a software only structure
 * and not part of the TDX module/VMM ABI.
 */
struct ve_info {
	u64 exit_reason;
	u64 exit_qual;
	/* Guest Linear (virtual) Address */
	u64 gla;
	/* Guest Physical Address */
	u64 gpa;
	u32 instr_len;
	u32 instr_info;
};

#ifdef CONFIG_INTEL_TDX_GUEST

extern int tdx_notify_irq;

void __init tdx_early_init(void);
bool tdx_debug_enabled(void);

void tdx_get_ve_info(struct ve_info *ve);

void __init tdx_filter_init(void);

bool tdx_handle_virt_exception(struct pt_regs *regs, struct ve_info *ve);

void tdx_safe_halt(void);

bool tdx_early_handle_ve(struct pt_regs *regs);

int tdx_mcall_get_report0(u8 *reportdata, u8 *tdreport);

bool tdx_allowed_port(int port);

u64 tdx_mcall_verify_report(u8 *reportmac);

int tdx_mcall_extend_rtmr(u8 *data, u8 index);

int tdx_hcall_get_quote(void *tdquote, int size);

int tdx_alloc_event_irq(void);

void tdx_free_event_irq(int);

/* Update the trace point symbolic printing too */
enum tdx_fuzz_loc {
	TDX_FUZZ_MSR_READ,
	TDX_FUZZ_MMIO_READ,
	TDX_FUZZ_PORT_IN,
	TDX_FUZZ_CPUID1,
	TDX_FUZZ_CPUID2,
	TDX_FUZZ_CPUID3,
	TDX_FUZZ_CPUID4,
	TDX_FUZZ_MSR_READ_ERR,
	TDX_FUZZ_MSR_WRITE_ERR,
	TDX_FUZZ_MAP_ERR,
	TDX_FUZZ_PORT_IN_ERR,
	TDX_FUZZ_VIRTIO,
	TDX_FUZZ_RANDOM,  /* kAFL */
	TDX_FUZZ_DEBUGFS, /* kAFL */
	TDX_FUZZ_MAX
};

#if defined(CONFIG_TDX_FUZZ) || defined(CONFIG_TDX_FUZZ_KAFL)
u64 tdx_fuzz(u64 var, uintptr_t addr, int size, enum tdx_fuzz_loc loc);
bool tdx_fuzz_err(enum tdx_fuzz_loc loc);
#else
static inline u64 tdx_fuzz(u64 var, uintptr_t addr, int size, enum tdx_fuzz_loc loc) { return var; };
static inline bool tdx_fuzz_err(enum tdx_fuzz_loc loc) { return false; }
#endif

#else

static inline void tdx_early_init(void) { };
static inline void tdx_safe_halt(void) { };
static inline void tdx_filter_init(void) { };

static inline bool tdx_early_handle_ve(struct pt_regs *regs) { return false; }

#endif /* CONFIG_INTEL_TDX_GUEST */

#if defined(CONFIG_KVM_GUEST) && defined(CONFIG_INTEL_TDX_GUEST)
long tdx_kvm_hypercall(unsigned int nr, unsigned long p1, unsigned long p2,
		       unsigned long p3, unsigned long p4);
#else
static inline long tdx_kvm_hypercall(unsigned int nr, unsigned long p1,
				     unsigned long p2, unsigned long p3,
				     unsigned long p4)
{
	return -ENODEV;
}
#endif /* CONFIG_INTEL_TDX_GUEST && CONFIG_KVM_GUEST */
#endif /* !__ASSEMBLY__ */
#endif /* _ASM_X86_TDX_H */
