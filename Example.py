import time
import timeit
from CmdsExple import SwitchInterface
import struct

switch = SwitchInterface()
while not switch.try_connect():
    print("Failed to connect.")
    print("Attempting to reconnect in 5 seconds...")
    time.sleep(5)
print("Connected to Switch Successfully!")

# if it ends here i know the next command didnt work, which is getmain
print("print1")
main = switch.get_main()
print("print2")
print(main)
print("print3")

# big ass thanks to Analog Guy on discord for all the python!
# this peeks through a pointer, keep in mind we can only look at addresses in our game
# def read_pointer():
# pos_ptr = switch.ptr_from_chain(main, (0xoffset1, offset2, 0x0ffset3, 0xoffset4, 0xoffset5))
# return struct.unpack("<3I", switch.peek(pos_ptr, 12))


print("print4")
#print(f"Reading pos: {timeit.timeit(read_pos, number=100)}")
print("print5?")
# this can be used to benchmark the sysmod, but it is already fast
# go ahead if you want to test though
# print(f"Reading a byte: {timeit.timeit(lambda: switch.peek(main, 2400), number=1000)}")
print("print6")
# big ass thanks to Analog Guy on discord again!
