from m5.params import *
from MemObject import MemObject

class Bus(MemObject):
    type = 'Bus'
    port = VectorPort("vector port for connecting devices")
    default = Port("Default port for requests that aren't handeled by a device.")
    bus_id = Param.Int(0, "blah")
    clock = Param.Clock("1GHz", "bus clock speed")
    width = Param.Int(64, "bus width (bytes)")
