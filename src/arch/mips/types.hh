/*
 * Copyright (c) 2007 MIPS Technologies, Inc.
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
 * Authors: Korey Sewell
 */

#ifndef __ARCH_MIPS_TYPES_HH__
#define __ARCH_MIPS_TYPES_HH__

#include "base/types.hh"

namespace MipsISA
{
    typedef uint32_t MachInst;
    typedef uint64_t ExtMachInst;
    typedef uint16_t  RegIndex;

    typedef uint32_t IntReg;
    typedef uint64_t LargestRead;


    // floating point register file entry type
    typedef uint32_t FloatReg32;
    typedef uint64_t FloatReg64;
    typedef uint64_t FloatRegBits;

    typedef double FloatRegVal;
    typedef double FloatReg;

    // cop-0/cop-1 system control register
    typedef uint64_t MiscReg;

    typedef union {
        IntReg   intreg;
        FloatReg fpreg;
        MiscReg  ctrlreg;
    } AnyReg;

    //used in FP convert & round function
    enum ConvertType{
        SINGLE_TO_DOUBLE,
        SINGLE_TO_WORD,
        SINGLE_TO_LONG,

        DOUBLE_TO_SINGLE,
        DOUBLE_TO_WORD,
        DOUBLE_TO_LONG,

        LONG_TO_SINGLE,
        LONG_TO_DOUBLE,
        LONG_TO_WORD,
        LONG_TO_PS,

        WORD_TO_SINGLE,
        WORD_TO_DOUBLE,
        WORD_TO_LONG,
        WORD_TO_PS,

        PL_TO_SINGLE,
        PU_TO_SINGLE
    };

    //used in FP convert & round function
    enum RoundMode{
        RND_ZERO,
        RND_DOWN,
        RND_UP,
        RND_NEAREST
   };

struct CoreSpecific {
      /* Note: It looks like it will be better to allow simulator users
         to specify the values of individual variables instead of requiring
         users to define the values of entire registers
         Especially since a lot of these variables can be created from other
         user parameters  (cache descriptions)
                                               -jpp
      */
      // MIPS CP0 State - First individual variables
      // Page numbers refer to revision 2.50 (July 2005) of the MIPS32 ARM, Volume III (PRA)
      unsigned CP0_IntCtl_IPTI; // Page 93, IP Timer Interrupt
      unsigned CP0_IntCtl_IPPCI; // Page 94, IP Performance Counter Interrupt
      unsigned CP0_SrsCtl_HSS; // Page 95, Highest Implemented Shadow Set
      unsigned CP0_PRId_CompanyOptions; // Page 105, Manufacture options
      unsigned CP0_PRId_CompanyID; // Page 105, Company ID - (0-255, 1=>MIPS)
      unsigned CP0_PRId_ProcessorID; // Page 105
      unsigned CP0_PRId_Revision; // Page 105
      unsigned CP0_EBase_CPUNum; // Page 106, CPU Number in a multiprocessor system
      unsigned CP0_Config_BE; // Page 108, Big/Little Endian mode
      unsigned CP0_Config_AT; //Page 109
      unsigned CP0_Config_AR; //Page 109
      unsigned CP0_Config_MT; //Page 109
      unsigned CP0_Config_VI; //Page 109
      unsigned CP0_Config1_M; // Page 110
      unsigned CP0_Config1_MMU; // Page 110
      unsigned CP0_Config1_IS; // Page 110
      unsigned CP0_Config1_IL; // Page 111
      unsigned CP0_Config1_IA; // Page 111
      unsigned CP0_Config1_DS; // Page 111
      unsigned CP0_Config1_DL; // Page 112
      unsigned CP0_Config1_DA; // Page 112
      bool CP0_Config1_C2; // Page 112
      bool CP0_Config1_MD;// Page 112 - Technically not used in MIPS32
      bool CP0_Config1_PC;// Page 112
      bool CP0_Config1_WR;// Page 113
      bool CP0_Config1_CA;// Page 113
      bool CP0_Config1_EP;// Page 113
      bool CP0_Config1_FP;// Page 113
      bool CP0_Config2_M; // Page 114
      unsigned CP0_Config2_TU;// Page 114
      unsigned CP0_Config2_TS;// Page 114
      unsigned CP0_Config2_TL;// Page 115
      unsigned CP0_Config2_TA;// Page 115
      unsigned CP0_Config2_SU;// Page 115
      unsigned CP0_Config2_SS;// Page 115
      unsigned CP0_Config2_SL;// Page 116
      unsigned CP0_Config2_SA;// Page 116
      bool CP0_Config3_M; //// Page 117
      bool CP0_Config3_DSPP;// Page 117
      bool CP0_Config3_LPA;// Page 117
      bool CP0_Config3_VEIC;// Page 118
      bool CP0_Config3_VInt; // Page 118
      bool CP0_Config3_SP;// Page 118
      bool CP0_Config3_MT;// Page 119
      bool CP0_Config3_SM;// Page 119
      bool CP0_Config3_TL;// Page 119

      bool CP0_WatchHi_M; // Page 124
      bool CP0_PerfCtr_M; // Page 130
      bool CP0_PerfCtr_W; // Page 130


      // Then, whole registers
      unsigned CP0_PRId;
      unsigned CP0_Config;
      unsigned CP0_Config1;
      unsigned CP0_Config2;
      unsigned CP0_Config3;
};

} // namespace MipsISA

#endif
