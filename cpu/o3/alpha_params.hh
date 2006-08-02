/*
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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
 */

#ifndef __CPU_O3_ALPHA_PARAMS_HH__
#define __CPU_O3_ALPHA_PARAMS_HH__

#include "cpu/o3/cpu.hh"

//Forward declarations
class AlphaDTB;
class AlphaITB;
class FUPool;
class FunctionalMemory;
class MemInterface;
class Process;
class System;

/**
 * This file defines the parameters that will be used for the AlphaFullCPU.
 * This must be defined externally so that the Impl can have a params class
 * defined that it can pass to all of the individual stages.
 */

class AlphaSimpleParams : public BaseFullCPU::Params
{
  public:

#if FULL_SYSTEM
    AlphaITB *itb; AlphaDTB *dtb;
#else
    std::vector<Process *> workload;
    Process *process;
#endif // FULL_SYSTEM

    //Page Table
//    PageTable *pTable;

    FunctionalMemory *mem;

    BaseCPU *checker;

    unsigned activity;

    //
    // Caches
    //
    MemInterface *icacheInterface;
    MemInterface *dcacheInterface;

    unsigned cachePorts;

    //
    // Fetch
    //
    unsigned decodeToFetchDelay;
    unsigned renameToFetchDelay;
    unsigned iewToFetchDelay;
    unsigned commitToFetchDelay;
    unsigned fetchWidth;

    //
    // Decode
    //
    unsigned renameToDecodeDelay;
    unsigned iewToDecodeDelay;
    unsigned commitToDecodeDelay;
    unsigned fetchToDecodeDelay;
    unsigned decodeWidth;

    //
    // Rename
    //
    unsigned iewToRenameDelay;
    unsigned commitToRenameDelay;
    unsigned decodeToRenameDelay;
    unsigned renameWidth;

    //
    // IEW
    //
    unsigned commitToIEWDelay;
    unsigned renameToIEWDelay;
    unsigned issueToExecuteDelay;
    unsigned dispatchWidth;
    unsigned issueWidth;
    unsigned wbWidth;
    unsigned wbDepth;
    FUPool *fuPool;

    //
    // Commit
    //
    unsigned iewToCommitDelay;
    unsigned renameToROBDelay;
    unsigned commitWidth;
    unsigned squashWidth;
    Tick trapLatency;
    Tick fetchTrapLatency;

    //
    // Timebuffer sizes
    //
    unsigned backComSize;
    unsigned forwardComSize;

    //
    // Branch predictor (BP, BTB, RAS)
    //
    std::string predType;
    unsigned localPredictorSize;
    unsigned localCtrBits;
    unsigned localHistoryTableSize;
    unsigned localHistoryBits;
    unsigned globalPredictorSize;
    unsigned globalCtrBits;
    unsigned globalHistoryBits;
    unsigned choicePredictorSize;
    unsigned choiceCtrBits;

    unsigned BTBEntries;
    unsigned BTBTagSize;

    unsigned RASSize;

    //
    // Load store queue
    //
    unsigned LQEntries;
    unsigned SQEntries;

    //
    // Memory dependence
    //
    unsigned SSITSize;
    unsigned LFSTSize;

    //
    // Miscellaneous
    //
    unsigned numPhysIntRegs;
    unsigned numPhysFloatRegs;
    unsigned numIQEntries;
    unsigned numROBEntries;

    //SMT Parameters
    unsigned smtNumFetchingThreads;

    std::string   smtFetchPolicy;

    std::string   smtIQPolicy;
    unsigned smtIQThreshold;

    std::string   smtLSQPolicy;
    unsigned smtLSQThreshold;

    std::string   smtCommitPolicy;

    std::string   smtROBPolicy;
    unsigned smtROBThreshold;

    // Probably can get this from somewhere.
    unsigned instShiftAmt;
};

#endif // __CPU_O3_ALPHA_PARAMS_HH__
