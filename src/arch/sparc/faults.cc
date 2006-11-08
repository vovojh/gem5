/*
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Gabe Black
 *          Kevin Lim
 */

#include <algorithm>

#include "arch/sparc/faults.hh"
#include "arch/sparc/isa_traits.hh"
#include "arch/sparc/process.hh"
#include "base/bitfield.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"
#if !FULL_SYSTEM
#include "mem/page_table.hh"
#include "sim/process.hh"
#endif

using namespace std;

namespace SparcISA
{

template<> SparcFaultBase::FaultVals
    SparcFault<InternalProcessorError>::vals = {"intprocerr", 0x029, 4};

template<> SparcFaultBase::FaultVals
    SparcFault<MemAddressNotAligned>::vals = {"unalign", 0x034, 10};

template<> SparcFaultBase::FaultVals
    SparcFault<PowerOnReset>::vals = {"pow_reset", 0x001, 0};

template<> SparcFaultBase::FaultVals
    SparcFault<WatchDogReset>::vals = {"watch_dog_reset", 0x002, 1};

template<> SparcFaultBase::FaultVals
    SparcFault<ExternallyInitiatedReset>::vals = {"extern_reset", 0x003, 1};

template<> SparcFaultBase::FaultVals
    SparcFault<SoftwareInitiatedReset>::vals = {"software_reset", 0x004, 1};

template<> SparcFaultBase::FaultVals
    SparcFault<REDStateException>::vals = {"red_counte", 0x005, 1};

template<> SparcFaultBase::FaultVals
    SparcFault<InstructionAccessException>::vals = {"inst_access", 0x008, 5};

template<> SparcFaultBase::FaultVals
    SparcFault<InstructionAccessMMUMiss>::vals = {"inst_mmu", 0x009, 2};

template<> SparcFaultBase::FaultVals
    SparcFault<InstructionAccessError>::vals = {"inst_error", 0x00A, 3};

template<> SparcFaultBase::FaultVals
    SparcFault<IllegalInstruction>::vals = {"illegal_inst", 0x010, 7};

template<> SparcFaultBase::FaultVals
    SparcFault<PrivilegedOpcode>::vals = {"priv_opcode", 0x011, 6};

template<> SparcFaultBase::FaultVals
    SparcFault<UnimplementedLDD>::vals = {"unimp_ldd", 0x012, 6};

template<> SparcFaultBase::FaultVals
    SparcFault<UnimplementedSTD>::vals = {"unimp_std", 0x013, 6};

template<> SparcFaultBase::FaultVals
    SparcFault<FpDisabled>::vals = {"fp_disabled", 0x020, 8};

template<> SparcFaultBase::FaultVals
    SparcFault<FpExceptionIEEE754>::vals = {"fp_754", 0x021, 11};

template<> SparcFaultBase::FaultVals
    SparcFault<FpExceptionOther>::vals = {"fp_other", 0x022, 11};

template<> SparcFaultBase::FaultVals
    SparcFault<TagOverflow>::vals = {"tag_overflow", 0x023, 14};

template<> SparcFaultBase::FaultVals
    SparcFault<DivisionByZero>::vals = {"div_by_zero", 0x028, 15};

template<> SparcFaultBase::FaultVals
    SparcFault<DataAccessException>::vals = {"data_access", 0x030, 12};

template<> SparcFaultBase::FaultVals
    SparcFault<DataAccessMMUMiss>::vals = {"data_mmu", 0x031, 12};

template<> SparcFaultBase::FaultVals
    SparcFault<DataAccessError>::vals = {"data_error", 0x032, 12};

template<> SparcFaultBase::FaultVals
    SparcFault<DataAccessProtection>::vals = {"data_protection", 0x033, 12};

template<> SparcFaultBase::FaultVals
    SparcFault<LDDFMemAddressNotAligned>::vals = {"unalign_lddf", 0x035, 10};

template<> SparcFaultBase::FaultVals
    SparcFault<STDFMemAddressNotAligned>::vals = {"unalign_stdf", 0x036, 10};

template<> SparcFaultBase::FaultVals
    SparcFault<PrivilegedAction>::vals = {"priv_action", 0x037, 11};

template<> SparcFaultBase::FaultVals
    SparcFault<LDQFMemAddressNotAligned>::vals = {"unalign_ldqf", 0x038, 10};

template<> SparcFaultBase::FaultVals
    SparcFault<STQFMemAddressNotAligned>::vals = {"unalign_stqf", 0x039, 10};

template<> SparcFaultBase::FaultVals
    SparcFault<AsyncDataError>::vals = {"async_data", 0x040, 2};

template<> SparcFaultBase::FaultVals
    SparcFault<CleanWindow>::vals = {"clean_win", 0x024, 10};

//The enumerated faults

template<> SparcFaultBase::FaultVals
    SparcFault<InterruptLevelN>::vals = {"interrupt_n", 0x041, 0};

template<> SparcFaultBase::FaultVals
    SparcFault<SpillNNormal>::vals = {"spill_n_normal", 0x080, 9};

template<> SparcFaultBase::FaultVals
    SparcFault<SpillNOther>::vals = {"spill_n_other", 0x0A0, 9};

template<> SparcFaultBase::FaultVals
    SparcFault<FillNNormal>::vals = {"fill_n_normal", 0x0C0, 9};

template<> SparcFaultBase::FaultVals
    SparcFault<FillNOther>::vals = {"fill_n_other", 0x0E0, 9};

template<> SparcFaultBase::FaultVals
    SparcFault<TrapInstruction>::vals = {"trap_inst_n", 0x100, 16};

#if !FULL_SYSTEM
template<> SparcFaultBase::FaultVals
    SparcFault<PageTableFault>::vals = {"page_table_fault", 0x0000, 0};
#endif

/**
 * This sets everything up for a normal trap except for actually jumping to
 * the handler. It will need to be expanded to include the state machine in
 * the manual. Right now it assumes that traps will always be to the
 * privileged level.
 */

void doNormalFault(ThreadContext *tc, TrapType tt)
{
    uint64_t TL = tc->readMiscReg(MISCREG_TL);
    uint64_t TSTATE = tc->readMiscReg(MISCREG_TSTATE);
    uint64_t PSTATE = tc->readMiscReg(MISCREG_PSTATE);
    uint64_t HPSTATE = tc->readMiscReg(MISCREG_HPSTATE);
    uint64_t CCR = tc->readMiscReg(MISCREG_CCR);
    uint64_t ASI = tc->readMiscReg(MISCREG_ASI);
    uint64_t CWP = tc->readMiscReg(MISCREG_CWP);
    uint64_t CANSAVE = tc->readMiscReg(MISCREG_CANSAVE);
    uint64_t GL = tc->readMiscReg(MISCREG_GL);
    uint64_t PC = tc->readPC();
    uint64_t NPC = tc->readNextPC();

    //Increment the trap level
    TL++;
    tc->setMiscReg(MISCREG_TL, TL);

    //Save off state

    //set TSTATE.gl to gl
    replaceBits(TSTATE, 42, 40, GL);
    //set TSTATE.ccr to ccr
    replaceBits(TSTATE, 39, 32, CCR);
    //set TSTATE.asi to asi
    replaceBits(TSTATE, 31, 24, ASI);
    //set TSTATE.pstate to pstate
    replaceBits(TSTATE, 20, 8, PSTATE);
    //set TSTATE.cwp to cwp
    replaceBits(TSTATE, 4, 0, CWP);

    //Write back TSTATE
    tc->setMiscReg(MISCREG_TSTATE, TSTATE);

    //set TPC to PC
    tc->setMiscReg(MISCREG_TPC, PC);
    //set TNPC to NPC
    tc->setMiscReg(MISCREG_TNPC, NPC);

    //set HTSTATE.hpstate to hpstate
    tc->setMiscReg(MISCREG_HTSTATE, HPSTATE);

    //TT = trap type;
    tc->setMiscReg(MISCREG_TT, tt);

    //Update the global register level
    if(1/*We're delivering the trap in priveleged mode*/)
        tc->setMiscReg(MISCREG_GL, max<int>(GL+1, MaxGL));
    else
        tc->setMiscReg(MISCREG_GL, max<int>(GL+1, MaxPGL));

    //PSTATE.mm is unchanged
    //PSTATE.pef = whether or not an fpu is present
    //XXX We'll say there's one present, even though there aren't
    //implementations for a decent number of the instructions
    PSTATE |= (1 << 4);
    //PSTATE.am = 0
    PSTATE &= ~(1 << 3);
    if(1/*We're delivering the trap in priveleged mode*/)
    {
        //PSTATE.priv = 1
        PSTATE |= (1 << 2);
        //PSTATE.cle = PSTATE.tle
        replaceBits(PSTATE, 9, 9, PSTATE >> 8);
    }
    else
    {
        //PSTATE.priv = 0
        PSTATE &= ~(1 << 2);
        //PSTATE.cle = 0
        PSTATE &= ~(1 << 9);
    }
    //PSTATE.ie = 0
    PSTATE &= ~(1 << 1);
    //PSTATE.tle is unchanged
    //PSTATE.tct = 0
    //XXX Where exactly is this field?
    tc->setMiscReg(MISCREG_PSTATE, PSTATE);

    if(0/*We're delivering the trap in hyperprivileged mode*/)
    {
        //HPSTATE.red = 0
        HPSTATE &= ~(1 << 5);
        //HPSTATE.hpriv = 1
        HPSTATE |= (1 << 2);
        //HPSTATE.ibe = 0
        HPSTATE &= ~(1 << 10);
        //HPSTATE.tlz is unchanged
        tc->setMiscReg(MISCREG_HPSTATE, HPSTATE);
    }

    bool changedCWP = true;
    if(tt == 0x24)
        CWP++;
    else if(0x80 <= tt && tt <= 0xbf)
        CWP += (CANSAVE + 2);
    else if(0xc0 <= tt && tt <= 0xff)
        CWP--;
    else
        changedCWP = false;

    if(changedCWP)
    {
        CWP = (CWP + NWindows) % NWindows;
        tc->setMiscRegWithEffect(MISCREG_CWP, CWP);
    }
}

#if FULL_SYSTEM

void SparcFaultBase::invoke(ThreadContext * tc)
{
    FaultBase::invoke(tc);
    countStat()++;

    //Use the SPARC trap state machine
    /*// exception restart address
    if (setRestartAddress() || !tc->inPalMode())
        tc->setMiscReg(AlphaISA::IPR_EXC_ADDR, tc->regs.pc);

    if (skipFaultingInstruction()) {
        // traps...  skip faulting instruction.
        tc->setMiscReg(AlphaISA::IPR_EXC_ADDR,
                   tc->readMiscReg(AlphaISA::IPR_EXC_ADDR) + 4);
    }

    if (!tc->inPalMode())
        AlphaISA::swap_palshadow(&(tc->regs), true);

    tc->regs.pc = tc->readMiscReg(AlphaISA::IPR_PAL_BASE) + vect();
    tc->regs.npc = tc->regs.pc + sizeof(MachInst);*/
}

#endif

#if !FULL_SYSTEM

void TrapInstruction::invoke(ThreadContext * tc)
{
    // Should be handled in ISA.
}

void SpillNNormal::invoke(ThreadContext *tc)
{
    doNormalFault(tc, trapType());

    Process *p = tc->getProcessPtr();

    //This will only work in faults from a SparcLiveProcess
    SparcLiveProcess *lp = dynamic_cast<SparcLiveProcess *>(p);
    assert(lp);

    //Then adjust the PC and NPC
    Addr spillStart = lp->readSpillStart();
    tc->setPC(spillStart);
    tc->setNextPC(spillStart + sizeof(MachInst));
    tc->setNextNPC(spillStart + 2*sizeof(MachInst));
}

void FillNNormal::invoke(ThreadContext *tc)
{
    doNormalFault(tc, trapType());

    Process * p = tc->getProcessPtr();

    //This will only work in faults from a SparcLiveProcess
    SparcLiveProcess *lp = dynamic_cast<SparcLiveProcess *>(p);
    assert(lp);

    //The adjust the PC and NPC
    Addr fillStart = lp->readFillStart();
    tc->setPC(fillStart);
    tc->setNextPC(fillStart + sizeof(MachInst));
    tc->setNextNPC(fillStart + 2*sizeof(MachInst));
}

void PageTableFault::invoke(ThreadContext *tc)
{
    Process *p = tc->getProcessPtr();

    // address is higher than the stack region or in the current stack region
    if (vaddr > p->stack_base || vaddr > p->stack_min)
        FaultBase::invoke(tc);

    // We've accessed the next page
    if (vaddr > p->stack_min - PageBytes) {
        p->stack_min -= PageBytes;
        if (p->stack_base - p->stack_min > 8*1024*1024)
            fatal("Over max stack size for one thread\n");
        p->pTable->allocate(p->stack_min, PageBytes);
        warn("Increasing stack size by one page.");
    } else {
        FaultBase::invoke(tc);
    }
}

#endif

} // namespace SparcISA

