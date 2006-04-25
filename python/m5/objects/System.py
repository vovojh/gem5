from m5 import *

class System(SimObject):
    type = 'System'
    boot_cpu_frequency = Param.Frequency(Self.cpu[0].clock.frequency,
                                         "boot processor frequency")
    memctrl = Param.MemoryController(Parent.any, "memory controller")
    physmem = Param.PhysicalMemory(Parent.any, "phsyical memory")
    init_param = Param.UInt64(0, "numerical value to pass into simulator")
    bin = Param.Bool(False, "is this system binned")
    binned_fns = VectorParam.String([], "functions broken down and binned")
    kernel = Param.String("file that contains the kernel code")
    readfile = Param.String("", "file to read startup script from")

class AlphaSystem(System):
    type = 'AlphaSystem'
    console = Param.String("file that contains the console code")
    pal = Param.String("file that contains palcode")
    boot_osflags = Param.String("a", "boot flags to pass to the kernel")
    system_type = Param.UInt64("Type of system we are emulating")
    system_rev = Param.UInt64("Revision of system we are emulating")
