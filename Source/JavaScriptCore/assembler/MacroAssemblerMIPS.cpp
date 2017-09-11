/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2017 Igalia, S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(ASSEMBLER)
#include "MacroAssembler.h"

#include "ProbeContext.h"

namespace JSC {

#if ENABLE(MASM_PROBE)

#define PROBE_SIZE 0 // FIXME
#define OUT_SIZE 0 // FIXME

extern "C" void ctiMasmProbeTrampoline();

using namespace MIPSRegisters;

struct IncomingProbeRecord {
    uintptr_t a0;
    uintptr_t a1;
    uintptr_t a2;
    uintptr_t t8;
    uintptr_t t9;
    uintptr_t ra;
};

static_assert(!(sizeof(IncomingProbeRecord) & 7), "IncomingProbeRecord must be 8-byte aligned");

asm (
    ".text" "\n"
    ".globl " SYMBOL_STRING(ctiMasmProbeTrampoline) "\n"
    HIDE_SYMBOL(ctiMasmProbeTrampoline) "\n"
    SYMBOL_STRING(ctiMasmProbeTrampoline) ":" "\n"

    // MacroAssembler::probe() has already generated code to store some values
    // in an IncomingProbeRecord. sp points to the IncomingProbeRecord.
    //
    // Incoming register values:
    // a0: probe function
    // a1: probe arg
    // a2: Probe::executeProbe
    // t8: scratch
    // t9: scratch, was ctiMasmProbeTrampoline
    // ra: previous return address

    "move       $t9, $sp" "\n"
    "sw         $a2, -" STRINGIZE_VALUE_OF(PTR_SIZE) "($t9)" "\n" // Stash Probe::executeProbe.
    "addiu      $a2, $a2, -" STRINGIZE_VALUE_OF(PROBE_SIZE + OUT_SIZE) "\n"
    // make sure the stack is 8 byte aligned
    "lui        $t8, 0xffff" "\n"
    "ori        $t8, 0xfff8" "\n"
    "and        $a2, $a2, $t8" "\n"
    "move       $sp, $a2" "\n"
    "lw         $a2, -" STRINGIZE_VALUE_OF(PTR_SIZE) "($t9)" "\n" // Reload Probe::executeProbe.

    "sw         $a0, " STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "($sp)" "\n"
    "sw         $a1, " STRINGIZE_VALUE_OF(PROBE_ARG_OFFSET) "($sp)" "\n"
    "sw         $ra, " STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "($sp)" "\n"
    "addiu      $a0, $t9, "STRINGIZE_VALUE_OF(IN_SIZE) "\n"
    "sw         $a0, " STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "($sp)" "\n"

    "sw         $v0, " STRINGIZE_VALUE_OF(PROBE_CPU_V0_OFFSET) "($sp)" "\n"
    "sw         $v1, " STRINGIZE_VALUE_OF(PROBE_CPU_V1_OFFSET) "($sp)" "\n"
    "sw         $a3, " STRINGIZE_VALUE_OF(PROBE_CPU_A3_OFFSET) "($sp)" "\n"
    "sw         $t0, " STRINGIZE_VALUE_OF(PROBE_CPU_T0_OFFSET) "($sp)" "\n"
    "sw         $t1, " STRINGIZE_VALUE_OF(PROBE_CPU_T1_OFFSET) "($sp)" "\n"
    "sw         $t2, " STRINGIZE_VALUE_OF(PROBE_CPU_T2_OFFSET) "($sp)" "\n"
    "sw         $t3, " STRINGIZE_VALUE_OF(PROBE_CPU_T3_OFFSET) "($sp)" "\n"
    "sw         $t4, " STRINGIZE_VALUE_OF(PROBE_CPU_T4_OFFSET) "($sp)" "\n"
    "sw         $t5, " STRINGIZE_VALUE_OF(PROBE_CPU_T5_OFFSET) "($sp)" "\n"
    "sw         $t6, " STRINGIZE_VALUE_OF(PROBE_CPU_T6_OFFSET) "($sp)" "\n"
    "sw         $t7, " STRINGIZE_VALUE_OF(PROBE_CPU_T7_OFFSET) "($sp)" "\n"
    "sw         $s0, " STRINGIZE_VALUE_OF(PROBE_CPU_S0_OFFSET) "($sp)" "\n"
    "sw         $s1, " STRINGIZE_VALUE_OF(PROBE_CPU_S1_OFFSET) "($sp)" "\n"
    "sw         $s2, " STRINGIZE_VALUE_OF(PROBE_CPU_S2_OFFSET) "($sp)" "\n"
    "sw         $s3, " STRINGIZE_VALUE_OF(PROBE_CPU_S3_OFFSET) "($sp)" "\n"
    "sw         $s4, " STRINGIZE_VALUE_OF(PROBE_CPU_S4_OFFSET) "($sp)" "\n"
    "sw         $s5, " STRINGIZE_VALUE_OF(PROBE_CPU_S5_OFFSET) "($sp)" "\n"
    "sw         $s6, " STRINGIZE_VALUE_OF(PROBE_CPU_S6_OFFSET) "($sp)" "\n"
    "sw         $s7, " STRINGIZE_VALUE_OF(PROBE_CPU_S7_OFFSET) "($sp)" "\n"
    "sw         $gp, " STRINGIZE_VALUE_OF(PROBE_CPU_GP_OFFSET) "($sp)" "\n"
    "sw         $s8, " STRINGIZE_VALUE_OF(PROBE_CPU_S8_OFFSET) "($sp)" "\n"

    "lw         $t0, " STRINGIZE_VALUE_OF(IN_A0_OFFSET) "($sp)" "\n"
    "lw         $t1, " STRINGIZE_VALUE_OF(IN_A1_OFFSET) "($sp)" "\n"
    "lw         $t2, " STRINGIZE_VALUE_OF(IN_A2_OFFSET) "($sp)" "\n"
    "lw         $t3, " STRINGIZE_VALUE_OF(IN_T8_OFFSET) "($sp)" "\n"
    "lw         $t4, " STRINGIZE_VALUE_OF(IN_T9_OFFSET) "($sp)" "\n"
    "lw         $t5, " STRINGIZE_VALUE_OF(IN_RA_OFFSET) "($sp)" "\n"
    "sw         $t0, " STRINGIZE_VALUE_OF(PROBE_CPU_A0_OFFSET) "($sp)" "\n"
    "sw         $t1, " STRINGIZE_VALUE_OF(PROBE_CPU_A1_OFFSET) "($sp)" "\n"
    "sw         $t2, " STRINGIZE_VALUE_OF(PROBE_CPU_A2_OFFSET) "($sp)" "\n"
    "sw         $t3, " STRINGIZE_VALUE_OF(PROBE_CPU_T8_OFFSET) "($sp)" "\n"
    "sw         $t4, " STRINGIZE_VALUE_OF(PROBE_CPU_T9_OFFSET) "($sp)" "\n"
    "sw         $t5, " STRINGIZE_VALUE_OF(PROBE_CPU_RA_OFFSET) "($sp)" "\n"

    "sdc1       $f0, " STRINGIZE_VALUE_OF(PROBE_CPU_F0_OFFSET) "($sp)" "\n"
    "sdc1       $f1, " STRINGIZE_VALUE_OF(PROBE_CPU_F1_OFFSET) "($sp)" "\n"
    "sdc1       $f2, " STRINGIZE_VALUE_OF(PROBE_CPU_F2_OFFSET) "($sp)" "\n"
    "sdc1       $f3, " STRINGIZE_VALUE_OF(PROBE_CPU_F3_OFFSET) "($sp)" "\n"
    "sdc1       $f4, " STRINGIZE_VALUE_OF(PROBE_CPU_F4_OFFSET) "($sp)" "\n"
    "sdc1       $f5, " STRINGIZE_VALUE_OF(PROBE_CPU_F5_OFFSET) "($sp)" "\n"
    "sdc1       $f6, " STRINGIZE_VALUE_OF(PROBE_CPU_F6_OFFSET) "($sp)" "\n"
    "sdc1       $f7, " STRINGIZE_VALUE_OF(PROBE_CPU_F7_OFFSET) "($sp)" "\n"
    "sdc1       $f8, " STRINGIZE_VALUE_OF(PROBE_CPU_F8_OFFSET) "($sp)" "\n"
    "sdc1       $f9, " STRINGIZE_VALUE_OF(PROBE_CPU_F9_OFFSET) "($sp)" "\n"
    "sdc1       $f10, " STRINGIZE_VALUE_OF(PROBE_CPU_F10_OFFSET) "($sp)" "\n"
    "sdc1       $f11, " STRINGIZE_VALUE_OF(PROBE_CPU_F11_OFFSET) "($sp)" "\n"
    "sdc1       $f12, " STRINGIZE_VALUE_OF(PROBE_CPU_F12_OFFSET) "($sp)" "\n"
    "sdc1       $f13, " STRINGIZE_VALUE_OF(PROBE_CPU_F13_OFFSET) "($sp)" "\n"
    "sdc1       $f14, " STRINGIZE_VALUE_OF(PROBE_CPU_F14_OFFSET) "($sp)" "\n"
    "sdc1       $f15, " STRINGIZE_VALUE_OF(PROBE_CPU_F15_OFFSET) "($sp)" "\n"
    "sdc1       $f16, " STRINGIZE_VALUE_OF(PROBE_CPU_F16_OFFSET) "($sp)" "\n"
    "sdc1       $f17, " STRINGIZE_VALUE_OF(PROBE_CPU_F17_OFFSET) "($sp)" "\n"
    "sdc1       $f18, " STRINGIZE_VALUE_OF(PROBE_CPU_F18_OFFSET) "($sp)" "\n"
    "sdc1       $f19, " STRINGIZE_VALUE_OF(PROBE_CPU_F19_OFFSET) "($sp)" "\n"
    "sdc1       $f20, " STRINGIZE_VALUE_OF(PROBE_CPU_F20_OFFSET) "($sp)" "\n"
    "sdc1       $f21, " STRINGIZE_VALUE_OF(PROBE_CPU_F21_OFFSET) "($sp)" "\n"
    "sdc1       $f22, " STRINGIZE_VALUE_OF(PROBE_CPU_F22_OFFSET) "($sp)" "\n"
    "sdc1       $f23, " STRINGIZE_VALUE_OF(PROBE_CPU_F23_OFFSET) "($sp)" "\n"
    "sdc1       $f24, " STRINGIZE_VALUE_OF(PROBE_CPU_F24_OFFSET) "($sp)" "\n"
    "sdc1       $f25, " STRINGIZE_VALUE_OF(PROBE_CPU_F25_OFFSET) "($sp)" "\n"
    "sdc1       $f26, " STRINGIZE_VALUE_OF(PROBE_CPU_F26_OFFSET) "($sp)" "\n"
    "sdc1       $f27, " STRINGIZE_VALUE_OF(PROBE_CPU_F27_OFFSET) "($sp)" "\n"
    "sdc1       $f28, " STRINGIZE_VALUE_OF(PROBE_CPU_F28_OFFSET) "($sp)" "\n"
    "sdc1       $f29, " STRINGIZE_VALUE_OF(PROBE_CPU_F29_OFFSET) "($sp)" "\n"
    "sdc1       $f30, " STRINGIZE_VALUE_OF(PROBE_CPU_F30_OFFSET) "($sp)" "\n"
    "sdc1       $f31, " STRINGIZE_VALUE_OF(PROBE_CPU_F31_OFFSET) "($sp)" "\n"

    "move       $s0, $sp" "\n" // Save Probe::State in a callee-savec register.

    "move       $a0, $sp" "\n"
    "move       $t9, $a2" "\n"
    "jalr       $t9" "\n"
    "nop" "\n"


    // FIXME: steps to call intializeStack (if it exists):
    // 1. check if it exists (else goto end)
    // 2. if Probe::state is above result $sp, move it somewhere safe
    // 3. do the jalr


    // Restore stuff
    "move       $sp, $s0" "\n"
    "ldc1       $f0, " STRINGIZE_VALUE_OF(PROBE_CPU_F0_OFFSET) "($sp)" "\n"
    "ldc1       $f1, " STRINGIZE_VALUE_OF(PROBE_CPU_F1_OFFSET) "($sp)" "\n"
    "ldc1       $f2, " STRINGIZE_VALUE_OF(PROBE_CPU_F2_OFFSET) "($sp)" "\n"
    "ldc1       $f3, " STRINGIZE_VALUE_OF(PROBE_CPU_F3_OFFSET) "($sp)" "\n"
    "ldc1       $f4, " STRINGIZE_VALUE_OF(PROBE_CPU_F4_OFFSET) "($sp)" "\n"
    "ldc1       $f5, " STRINGIZE_VALUE_OF(PROBE_CPU_F5_OFFSET) "($sp)" "\n"
    "ldc1       $f6, " STRINGIZE_VALUE_OF(PROBE_CPU_F6_OFFSET) "($sp)" "\n"
    "ldc1       $f7, " STRINGIZE_VALUE_OF(PROBE_CPU_F7_OFFSET) "($sp)" "\n"
    "ldc1       $f8, " STRINGIZE_VALUE_OF(PROBE_CPU_F8_OFFSET) "($sp)" "\n"
    "ldc1       $f9, " STRINGIZE_VALUE_OF(PROBE_CPU_F9_OFFSET) "($sp)" "\n"
    "ldc1       $f10, " STRINGIZE_VALUE_OF(PROBE_CPU_F10_OFFSET) "($sp)" "\n"
    "ldc1       $f11, " STRINGIZE_VALUE_OF(PROBE_CPU_F11_OFFSET) "($sp)" "\n"
    "ldc1       $f12, " STRINGIZE_VALUE_OF(PROBE_CPU_F12_OFFSET) "($sp)" "\n"
    "ldc1       $f13, " STRINGIZE_VALUE_OF(PROBE_CPU_F13_OFFSET) "($sp)" "\n"
    "ldc1       $f14, " STRINGIZE_VALUE_OF(PROBE_CPU_F14_OFFSET) "($sp)" "\n"
    "ldc1       $f15, " STRINGIZE_VALUE_OF(PROBE_CPU_F15_OFFSET) "($sp)" "\n"
    "ldc1       $f16, " STRINGIZE_VALUE_OF(PROBE_CPU_F16_OFFSET) "($sp)" "\n"
    "ldc1       $f17, " STRINGIZE_VALUE_OF(PROBE_CPU_F17_OFFSET) "($sp)" "\n"
    "ldc1       $f18, " STRINGIZE_VALUE_OF(PROBE_CPU_F18_OFFSET) "($sp)" "\n"
    "ldc1       $f19, " STRINGIZE_VALUE_OF(PROBE_CPU_F19_OFFSET) "($sp)" "\n"
    "ldc1       $f20, " STRINGIZE_VALUE_OF(PROBE_CPU_F20_OFFSET) "($sp)" "\n"
    "ldc1       $f21, " STRINGIZE_VALUE_OF(PROBE_CPU_F21_OFFSET) "($sp)" "\n"
    "ldc1       $f22, " STRINGIZE_VALUE_OF(PROBE_CPU_F22_OFFSET) "($sp)" "\n"
    "ldc1       $f23, " STRINGIZE_VALUE_OF(PROBE_CPU_F23_OFFSET) "($sp)" "\n"
    "ldc1       $f24, " STRINGIZE_VALUE_OF(PROBE_CPU_F24_OFFSET) "($sp)" "\n"
    "ldc1       $f25, " STRINGIZE_VALUE_OF(PROBE_CPU_F25_OFFSET) "($sp)" "\n"
    "ldc1       $f26, " STRINGIZE_VALUE_OF(PROBE_CPU_F26_OFFSET) "($sp)" "\n"
    "ldc1       $f27, " STRINGIZE_VALUE_OF(PROBE_CPU_F27_OFFSET) "($sp)" "\n"
    "ldc1       $f28, " STRINGIZE_VALUE_OF(PROBE_CPU_F28_OFFSET) "($sp)" "\n"
    "ldc1       $f29, " STRINGIZE_VALUE_OF(PROBE_CPU_F29_OFFSET) "($sp)" "\n"
    "ldc1       $f30, " STRINGIZE_VALUE_OF(PROBE_CPU_F30_OFFSET) "($sp)" "\n"
    "ldc1       $f31, " STRINGIZE_VALUE_OF(PROBE_CPU_F31_OFFSET) "($sp)" "\n"

    "lw         $a0, " STRINGIZE_VALUE_OF(PROBE_CPU_A0_OFFSET) "($sp)" "\n"
    "lw         $a1, " STRINGIZE_VALUE_OF(PROBE_CPU_A1_OFFSET) "($sp)" "\n"
    "lw         $a2, " STRINGIZE_VALUE_OF(PROBE_CPU_A2_OFFSET) "($sp)" "\n"
    "lw         $a3, " STRINGIZE_VALUE_OF(PROBE_CPU_A3_OFFSET) "($sp)" "\n"
    "lw         $t0, " STRINGIZE_VALUE_OF(PROBE_CPU_T0_OFFSET) "($sp)" "\n"
    "lw         $t1, " STRINGIZE_VALUE_OF(PROBE_CPU_T1_OFFSET) "($sp)" "\n"
    "lw         $t2, " STRINGIZE_VALUE_OF(PROBE_CPU_T2_OFFSET) "($sp)" "\n"
    "lw         $t3, " STRINGIZE_VALUE_OF(PROBE_CPU_T3_OFFSET) "($sp)" "\n"
    "lw         $t4, " STRINGIZE_VALUE_OF(PROBE_CPU_T4_OFFSET) "($sp)" "\n"
    "lw         $t5, " STRINGIZE_VALUE_OF(PROBE_CPU_T5_OFFSET) "($sp)" "\n"
    "lw         $t6, " STRINGIZE_VALUE_OF(PROBE_CPU_T6_OFFSET) "($sp)" "\n"
    "lw         $t7, " STRINGIZE_VALUE_OF(PROBE_CPU_T7_OFFSET) "($sp)" "\n"
    "lw         $s0, " STRINGIZE_VALUE_OF(PROBE_CPU_S0_OFFSET) "($sp)" "\n"
    "lw         $s1, " STRINGIZE_VALUE_OF(PROBE_CPU_S1_OFFSET) "($sp)" "\n"
    "lw         $s2, " STRINGIZE_VALUE_OF(PROBE_CPU_S2_OFFSET) "($sp)" "\n"
    "lw         $s3, " STRINGIZE_VALUE_OF(PROBE_CPU_S3_OFFSET) "($sp)" "\n"
    "lw         $s4, " STRINGIZE_VALUE_OF(PROBE_CPU_S4_OFFSET) "($sp)" "\n"
    "lw         $s5, " STRINGIZE_VALUE_OF(PROBE_CPU_S5_OFFSET) "($sp)" "\n"
    "lw         $s6, " STRINGIZE_VALUE_OF(PROBE_CPU_S6_OFFSET) "($sp)" "\n"
    "lw         $s7, " STRINGIZE_VALUE_OF(PROBE_CPU_S7_OFFSET) "($sp)" "\n"
    "lw         $t8, " STRINGIZE_VALUE_OF(PROBE_CPU_T8_OFFSET) "($sp)" "\n"
    "lw         $t9, " STRINGIZE_VALUE_OF(PROBE_CPU_T9_OFFSET) "($sp)" "\n"

    // Just like on ARM64, we can only change pc via a branch or jalr, meaning
    // we need a free register, in that case, we'll use ra, and restore it in
    // the probe. Since we restore ra in the probe, ra cannot be restored if
    // the probe function changed the pc, and we have a RELEASE_ASSERT in
    // Probe::executeProbe to ascertain that.
);

void MacroAssembler::probe(Probe::Function function, void* arg)
{
    sub32(TrustedImm32(sizeof(IncomingRecord)), sp);
    store32(a0, Address(sp, offsetof(IncomingRecord, a0)));
    store32(a1, Address(sp, offsetof(IncomingRecord, a1)));
    store32(a2, Address(sp, offsetof(IncomingRecord, a2)));
    store32(t8, Address(sp, offsetof(IncomingRecord, t8)));
    store32(t9, Address(sp, offsetof(IncomingRecord, t9)));
    store32(ra, Address(sp, offsetof(IncomingRecord, ra)));

    move(TrustedImmPtr(reinterpret_cast<void*>(function)), a0);
    move(TrustedImmPtr(arg), a1);
    move(TrustedImmPtr(reinterpret_cast<void*>(Probe::executeProbe)), a2);
    move(TrustedImmPtr(reinterpret_cast<void*>(ctiMasmProbeTrampoline)), t9);
    m_assembler.jalr(t9);
    m_assembler.nop();
}

#endif // ENABLE(MASM_PROBE)

} // namespace JSC


#endif // ENABLE(ASSEMBLER)
