import struct
import usb.core
import usb.util


class SwitchInterface:
    def __init__(self):
        self._dev = None
        self._out = None
        self._in = None

    def try_connect(self, vendor_id=0x057E, product_id=0x3000):
        self._dev = usb.core.find(idVendor=vendor_id, idProduct=product_id)
        if self._dev is not None:
            try:
                self._dev.set_configuration()
                intf = self._dev.get_active_configuration()[(0, 0)]
                self._out = usb.util.find_descriptor(
                    intf,
                    custom_match=lambda e: usb.util.endpoint_direction(
                        e.bEndpointAddress
                    )
                    == usb.util.ENDPOINT_OUT,
                )
                self._in = usb.util.find_descriptor(
                    intf,
                    custom_match=lambda e: usb.util.endpoint_direction(
                        e.bEndpointAddress
                    )
                    == usb.util.ENDPOINT_IN,
                )
                return True
            except:
                return False
        else:
            return False

    def send_raw(self, content):
        self._out.write(struct.pack("<I", (len(content) + 2)))
        self._out.write(content)

    def read_raw(self):
        size = int(struct.unpack(
            "<L", self._in.read(4, timeout=0).tobytes())[0])
        return self._in.read(size, timeout=0).tobytes().ljust(size, b"\0")

    def peek(self, address, size):
        self.send_raw(f"peekAbsolute {hex(address)} {hex(size)}\n\r")
        return self.read_raw()

    def get_main(self):  # sends the getMainNsoBase command, and recieves the data back
        self.send_raw("getMainNsoBase\r\n")
        return SwitchInterface.bytes_to_int(self.read_raw(), False)

    def ptr_from_chain(self, main, chain):
        chain_iter = iter(chain)
        ptr = main + next(chain_iter)
        for offset in chain_iter:
            ptr = SwitchInterface.bytes_to_int(
                self.peek(ptr, 8), False) + offset
        return ptr

    @staticmethod
    def bytes_to_int(byte_arr, signed):
        return int.from_bytes(byte_arr, byteorder="little", signed=signed)
