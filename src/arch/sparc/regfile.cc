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
 *          Ali Saidi
 */

#include "arch/sparc/regfile.hh"
#include "cpu/thread_context.hh"

class Checkpoint;

using namespace SparcISA;
using namespace std;

//RegFile class methods
Addr RegFile::readPC()
{
    return pc;
}

void RegFile::setPC(Addr val)
{
    pc = val;
}

Addr RegFile::readNextPC()
{
    return npc;
}

void RegFile::setNextPC(Addr val)
{
    npc = val;
}

Addr RegFile::readNextNPC()
{
    return nnpc;
}

void RegFile::setNextNPC(Addr val)
{
    nnpc = val;
}

void RegFile::clear()
{
    intRegFile.clear();
    floatRegFile.clear();
}

MiscReg RegFile::readMiscReg(int miscReg)
{
    return miscRegFile.readReg(miscReg);
}

MiscReg RegFile::readMiscRegWithEffect(int miscReg,
        Fault &fault, ThreadContext *tc)
{
    return miscRegFile.readRegWithEffect(miscReg, fault, tc);
}

Fault RegFile::setMiscReg(int miscReg, const MiscReg &val)
{
    return miscRegFile.setReg(miscReg, val);
}

Fault RegFile::setMiscRegWithEffect(int miscReg, const MiscReg &val,
        ThreadContext * tc)
{
    return miscRegFile.setRegWithEffect(miscReg, val, tc);
}

FloatReg RegFile::readFloatReg(int floatReg, int width)
{
    return floatRegFile.readReg(floatReg, width);
}

FloatReg RegFile::readFloatReg(int floatReg)
{
    //Use the "natural" width of a single float
    return floatRegFile.readReg(floatReg, FloatRegFile::SingleWidth);
}

FloatRegBits RegFile::readFloatRegBits(int floatReg, int width)
{
    return floatRegFile.readRegBits(floatReg, width);
}

FloatRegBits RegFile::readFloatRegBits(int floatReg)
{
    //Use the "natural" width of a single float
    return floatRegFile.readRegBits(floatReg,
            FloatRegFile::SingleWidth);
}

Fault RegFile::setFloatReg(int floatReg, const FloatReg &val, int width)
{
    return floatRegFile.setReg(floatReg, val, width);
}

Fault RegFile::setFloatReg(int floatReg, const FloatReg &val)
{
    //Use the "natural" width of a single float
    return setFloatReg(floatReg, val, FloatRegFile::SingleWidth);
}

Fault RegFile::setFloatRegBits(int floatReg, const FloatRegBits &val, int width)
{
    return floatRegFile.setRegBits(floatReg, val, width);
}

Fault RegFile::setFloatRegBits(int floatReg, const FloatRegBits &val)
{
    //Use the "natural" width of a single float
    return floatRegFile.setRegBits(floatReg, val,
            FloatRegFile::SingleWidth);
}

IntReg RegFile::readIntReg(int intReg)
{
    return intRegFile.readReg(intReg);
}

Fault RegFile::setIntReg(int intReg, const IntReg &val)
{
    return intRegFile.setReg(intReg, val);
}

void RegFile::serialize(std::ostream &os)
{
    intRegFile.serialize(os);
    floatRegFile.serialize(os);
    miscRegFile.serialize(os);
    SERIALIZE_SCALAR(pc);
    SERIALIZE_SCALAR(npc);
}

void RegFile::unserialize(Checkpoint *cp, const std::string &section)
{
    intRegFile.unserialize(cp, section);
    floatRegFile.unserialize(cp, section);
    miscRegFile.unserialize(cp, section);
    UNSERIALIZE_SCALAR(pc);
    UNSERIALIZE_SCALAR(npc);
}

void RegFile::changeContext(RegContextParam param, RegContextVal val)
{
    switch(param)
    {
      case CONTEXT_CWP:
        intRegFile.setCWP(val);
        break;
      case CONTEXT_GLOBALS:
        intRegFile.setGlobals(val);
        break;
      default:
        panic("Tried to set illegal context parameter in the SPARC regfile.\n");
    }
}

int SparcISA::InterruptLevel(uint64_t softint)
{
    if (softint & 0x10000 || softint & 0x1)
        return 14;

    int level = 14;
    while (level >= 0 && !(1 << (level + 1) & softint))
        level--;
    if (1 << (level + 1) & softint)
        return level;
    return 0;
}

void SparcISA::copyMiscRegs(ThreadContext *src, ThreadContext *dest)
{

    uint8_t tl = src->readMiscReg(MISCREG_TL);

    // Read all the trap level dependent registers and save them off
    for(int i = 1; i <= MaxTL; i++)
    {
        src->setMiscReg(MISCREG_TL, i);
        dest->setMiscReg(MISCREG_TL, i);

        dest->setMiscReg(MISCREG_TT, src->readMiscReg(MISCREG_TT));
        dest->setMiscReg(MISCREG_TPC, src->readMiscReg(MISCREG_TPC));
        dest->setMiscReg(MISCREG_TNPC, src->readMiscReg(MISCREG_TNPC));
        dest->setMiscReg(MISCREG_TSTATE, src->readMiscReg(MISCREG_TSTATE));
    }

    // Save off the traplevel
    dest->setMiscReg(MISCREG_TL, tl);
    src->setMiscReg(MISCREG_TL, tl);


    // ASRs
    dest->setMiscReg(MISCREG_Y, src->readMiscReg(MISCREG_Y));
    dest->setMiscReg(MISCREG_CCR, src->readMiscReg(MISCREG_CCR));
    dest->setMiscReg(MISCREG_ASI, src->readMiscReg(MISCREG_ASI));
    dest->setMiscReg(MISCREG_TICK, src->readMiscReg(MISCREG_TICK));
    dest->setMiscReg(MISCREG_FPRS, src->readMiscReg(MISCREG_FPRS));
    dest->setMiscReg(MISCREG_SOFTINT, src->readMiscReg(MISCREG_SOFTINT));
    dest->setMiscReg(MISCREG_TICK_CMPR, src->readMiscReg(MISCREG_TICK_CMPR));
    dest->setMiscReg(MISCREG_STICK, src->readMiscReg(MISCREG_STICK));
    dest->setMiscReg(MISCREG_STICK_CMPR, src->readMiscReg(MISCREG_STICK_CMPR));

    // Priv Registers
    dest->setMiscReg(MISCREG_TICK, src->readMiscReg(MISCREG_TICK));
    dest->setMiscReg(MISCREG_TBA, src->readMiscReg(MISCREG_TBA));
    dest->setMiscReg(MISCREG_PSTATE, src->readMiscReg(MISCREG_PSTATE));
    dest->setMiscReg(MISCREG_PIL, src->readMiscReg(MISCREG_PIL));
    dest->setMiscReg(MISCREG_CWP, src->readMiscReg(MISCREG_CWP));
    dest->setMiscReg(MISCREG_CANSAVE, src->readMiscReg(MISCREG_CANSAVE));
    dest->setMiscReg(MISCREG_CANRESTORE, src->readMiscReg(MISCREG_CANRESTORE));
    dest->setMiscReg(MISCREG_OTHERWIN, src->readMiscReg(MISCREG_OTHERWIN));
    dest->setMiscReg(MISCREG_CLEANWIN, src->readMiscReg(MISCREG_CLEANWIN));
    dest->setMiscReg(MISCREG_WSTATE, src->readMiscReg(MISCREG_WSTATE));
    dest->setMiscReg(MISCREG_GL, src->readMiscReg(MISCREG_GL));

    // Hyperprivilged registers
    dest->setMiscReg(MISCREG_HPSTATE, src->readMiscReg(MISCREG_HPSTATE));
    dest->setMiscReg(MISCREG_HINTP, src->readMiscReg(MISCREG_HINTP));
    dest->setMiscReg(MISCREG_HTBA, src->readMiscReg(MISCREG_HTBA));
    dest->setMiscReg(MISCREG_STRAND_STS_REG,
            src->readMiscReg(MISCREG_STRAND_STS_REG));
    dest->setMiscReg(MISCREG_HSTICK_CMPR,
            src->readMiscReg(MISCREG_HSTICK_CMPR));

    // FSR
    dest->setMiscReg(MISCREG_FSR, src->readMiscReg(MISCREG_FSR));
}

void SparcISA::copyRegs(ThreadContext *src, ThreadContext *dest)
{
    // First loop through the integer registers.
    for (int i = 0; i < TheISA::NumIntRegs; ++i) {
        dest->setIntReg(i, src->readIntReg(i));
    }

    // Then loop through the floating point registers.
    for (int i = 0; i < TheISA::NumFloatRegs; ++i) {
        dest->setFloatRegBits(i, src->readFloatRegBits(i));
    }

    // Copy misc. registers
    copyMiscRegs(src, dest);

    // Lastly copy PC/NPC
    dest->setPC(src->readPC());
    dest->setNextPC(src->readNextPC());
    dest->setNextNPC(src->readNextNPC());
}
