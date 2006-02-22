/*
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
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

#ifndef __DEV_SINIC_HH__
#define __DEV_SINIC_HH__

#include "base/inet.hh"
#include "base/statistics.hh"
#include "dev/etherint.hh"
#include "dev/etherpkt.hh"
#include "dev/io_device.hh"
#include "dev/pcidev.hh"
#include "dev/pktfifo.hh"
#include "dev/sinicreg.hh"
#include "mem/bus/bus.hh"
#include "sim/eventq.hh"

namespace Sinic {

class Interface;
class Base : public PciDev
{
  protected:
    bool rxEnable;
    bool txEnable;
    Tick clock;
    inline Tick cycles(int numCycles) const { return numCycles * clock; }

  protected:
    Tick intrDelay;
    Tick intrTick;
    bool cpuIntrEnable;
    bool cpuPendingIntr;
    void cpuIntrPost(Tick when);
    void cpuInterrupt();
    void cpuIntrClear();

    typedef EventWrapper<Base, &Base::cpuInterrupt> IntrEvent;
    friend void IntrEvent::process();
    IntrEvent *intrEvent;
    Interface *interface;

    bool cpuIntrPending() const;
    void cpuIntrAck() { cpuIntrClear(); }

/**
 * Serialization stuff
 */
  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

/**
 * Construction/Destruction/Parameters
 */
  public:
    struct Params : public PciDev::Params
    {
        Tick clock;
        Tick intr_delay;
    };

    Base(Params *p);
};

class Device : public Base
{
  protected:
    Platform *plat;
    PhysicalMemory *physmem;

  protected:
    /** Receive State Machine States */
    enum RxState {
        rxIdle,
        rxFifoBlock,
        rxBeginCopy,
        rxCopy,
        rxCopyDone
    };

    /** Transmit State Machine states */
    enum TxState {
        txIdle,
        txFifoBlock,
        txBeginCopy,
        txCopy,
        txCopyDone
    };

    /** device register file */
    struct {
        uint32_t Config;       // 0x00
        uint32_t Command;      // 0x04
        uint32_t IntrStatus;   // 0x08
        uint32_t IntrMask;     // 0x0c
        uint32_t RxMaxCopy;    // 0x10
        uint32_t TxMaxCopy;    // 0x14
        uint32_t RxMaxIntr;    // 0x18
        uint32_t Reserved0;    // 0x1c
        uint32_t RxFifoSize;   // 0x20
        uint32_t TxFifoSize;   // 0x24
        uint32_t RxFifoMark;   // 0x28
        uint32_t TxFifoMark;   // 0x2c
        uint64_t RxData;       // 0x30
        uint64_t RxDone;       // 0x38
        uint64_t RxWait;       // 0x40
        uint64_t TxData;       // 0x48
        uint64_t TxDone;       // 0x50
        uint64_t TxWait;       // 0x58
        uint64_t HwAddr;       // 0x60
    } regs;

    struct VirtualReg {
        uint64_t RxData;
        uint64_t RxDone;
        uint64_t TxData;
        uint64_t TxDone;

        PacketFifo::iterator rxPacket;
        int rxPacketOffset;
        int rxPacketBytes;
        uint64_t rxDoneData;

        VirtualReg()
            : RxData(0), RxDone(0), TxData(0), TxDone(0),
              rxPacketOffset(0), rxPacketBytes(0), rxDoneData(0)
        { }
    };
    typedef std::vector<VirtualReg> VirtualRegs;
    typedef std::list<int> VirtualList;
    VirtualRegs virtualRegs;
    VirtualList rxList;
    VirtualList txList;

    uint8_t  &regData8(Addr daddr) { return *((uint8_t *)&regs + daddr); }
    uint32_t &regData32(Addr daddr) { return *(uint32_t *)&regData8(daddr); }
    uint64_t &regData64(Addr daddr) { return *(uint64_t *)&regData8(daddr); }

  private:
    Addr addr;
    static const Addr size = Regs::Size;

  protected:
    RxState rxState;
    PacketFifo rxFifo;
    PacketFifo::iterator rxFifoPtr;
    bool rxEmpty;
    Addr rxDmaAddr;
    uint8_t *rxDmaData;
    int rxDmaLen;

    TxState txState;
    PacketFifo txFifo;
    bool txFull;
    PacketPtr txPacket;
    int txPacketOffset;
    int txPacketBytes;
    Addr txDmaAddr;
    uint8_t *txDmaData;
    int txDmaLen;

  protected:
    void reset();

    void rxKick();
    Tick rxKickTick;
    typedef EventWrapper<Device, &Device::rxKick> RxKickEvent;
    friend void RxKickEvent::process();

    void txKick();
    Tick txKickTick;
    typedef EventWrapper<Device, &Device::txKick> TxKickEvent;
    friend void TxKickEvent::process();

    /**
     * Retransmit event
     */
    void transmit();
    void txEventTransmit()
    {
        transmit();
        if (txState == txFifoBlock)
            txKick();
    }
    typedef EventWrapper<Device, &Device::txEventTransmit> TxEvent;
    friend void TxEvent::process();
    TxEvent txEvent;

    void txDump() const;
    void rxDump() const;

    /**
     * receive address filter
     */
    bool rxFilter(const PacketPtr &packet);

/**
 * device configuration
 */
    void changeConfig(uint32_t newconfig);
    void command(uint32_t command);

/**
 * device ethernet interface
 */
  public:
    bool recvPacket(PacketPtr packet);
    void transferDone();
    void setInterface(Interface *i) { assert(!interface); interface = i; }

/**
 * DMA parameters
 */
  protected:
    void rxDmaCopy();
    void rxDmaDone();
    friend class EventWrapper<Device, &Device::rxDmaDone>;
    EventWrapper<Device, &Device::rxDmaDone> rxDmaEvent;

    void txDmaCopy();
    void txDmaDone();
    friend class EventWrapper<Device, &Device::txDmaDone>;
    EventWrapper<Device, &Device::txDmaDone> txDmaEvent;

    Tick dmaReadDelay;
    Tick dmaReadFactor;
    Tick dmaWriteDelay;
    Tick dmaWriteFactor;

/**
 * Interrupt management
 */
  protected:
    void devIntrPost(uint32_t interrupts);
    void devIntrClear(uint32_t interrupts = Regs::Intr_All);
    void devIntrChangeMask(uint32_t newmask);

/**
 * PCI Configuration interface
 */
  public:
    virtual void writeConfig(int offset, int size, const uint8_t *data);

/**
 * Memory Interface
 */
  public:
    virtual Fault read(MemReqPtr &req, uint8_t *data);
    virtual Fault write(MemReqPtr &req, const uint8_t *data);

    void prepareIO(int cpu, int index);
    void prepareRead(int cpu, int index);
    void prepareWrite(int cpu, int index);
    Fault iprRead(Addr daddr, int cpu, uint64_t &result);
    Fault readBar0(MemReqPtr &req, Addr daddr, uint8_t *data);
    Fault writeBar0(MemReqPtr &req, Addr daddr, const uint8_t *data);
    void regWrite(Addr daddr, int cpu, const uint8_t *data);
    Tick cacheAccess(MemReqPtr &req);

/**
 * Statistics
 */
  private:
    Stats::Scalar<> rxBytes;
    Stats::Formula  rxBandwidth;
    Stats::Scalar<> rxPackets;
    Stats::Formula  rxPacketRate;
    Stats::Scalar<> rxIpPackets;
    Stats::Scalar<> rxTcpPackets;
    Stats::Scalar<> rxUdpPackets;
    Stats::Scalar<> rxIpChecksums;
    Stats::Scalar<> rxTcpChecksums;
    Stats::Scalar<> rxUdpChecksums;

    Stats::Scalar<> txBytes;
    Stats::Formula  txBandwidth;
    Stats::Formula totBandwidth;
    Stats::Formula totPackets;
    Stats::Formula totBytes;
    Stats::Formula totPacketRate;
    Stats::Scalar<> txPackets;
    Stats::Formula  txPacketRate;
    Stats::Scalar<> txIpPackets;
    Stats::Scalar<> txTcpPackets;
    Stats::Scalar<> txUdpPackets;
    Stats::Scalar<> txIpChecksums;
    Stats::Scalar<> txTcpChecksums;
    Stats::Scalar<> txUdpChecksums;

  public:
    virtual void regStats();

/**
 * Serialization stuff
 */
  public:
    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);

/**
 * Construction/Destruction/Parameters
 */
  public:
    struct Params : public Base::Params
    {
        IntrControl *i;
        PhysicalMemory *pmem;
        Tick tx_delay;
        Tick rx_delay;
        HierParams *hier;
        Bus *pio_bus;
        Bus *header_bus;
        Bus *payload_bus;
        Tick pio_latency;
        PhysicalMemory *physmem;
        IntrControl *intctrl;
        bool rx_filter;
        Net::EthAddr eaddr;
        uint32_t rx_max_copy;
        uint32_t tx_max_copy;
        uint32_t rx_max_intr;
        uint32_t rx_fifo_size;
        uint32_t tx_fifo_size;
        uint32_t rx_fifo_threshold;
        uint32_t tx_fifo_threshold;
        Tick dma_read_delay;
        Tick dma_read_factor;
        Tick dma_write_delay;
        Tick dma_write_factor;
        bool dma_no_allocate;
        bool rx_thread;
        bool tx_thread;
    };

  protected:
    const Params *params() const { return (const Params *)_params; }

  public:
    Device(Params *params);
    ~Device();
};

/*
 * Ethernet Interface for an Ethernet Device
 */
class Interface : public EtherInt
{
  private:
    Device *dev;

  public:
    Interface(const std::string &name, Device *d)
        : EtherInt(name), dev(d) { dev->setInterface(this); }

    virtual bool recvPacket(PacketPtr pkt) { return dev->recvPacket(pkt); }
    virtual void sendDone() { dev->transferDone(); }
};

/* namespace Sinic */ }

#endif // __DEV_SINIC_HH__
