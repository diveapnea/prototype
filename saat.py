import struct
import sys

if len(sys.argv) !=2:
   print "Wrong number of arguments. We need one epprom hex to decode"
   exit(1)

#reached FF area?
empty_reached = 0


with open(sys.argv[1], "rb") as f:
    byte = f.read(1)

    while byte != "":
	# day is unsigned byte
	print("{0:02d}.".format(struct.unpack('B', byte)[0])),
	byte = f.read(1)
	if byte != "":
	   #month is unsigned byte
	   print("{0:02d}.".format(struct.unpack('B', byte)[0])),
  	byte = f.read(1)
	if byte != "":
	   #year is unsigned byte
	   print("20{0:02d}".format(struct.unpack('B', byte)[0]))
  	byte = f.read(1)
	if byte != "":
	   #hour is unsigned byte
	   print("{}:".format(struct.unpack('B', byte)[0])),
	byte = f.read(1)
	if byte != "":
	   # minute is unsigned byte
	   print("{}:".format(struct.unpack('B', byte)[0])),
  	byte = f.read(1)
	if byte != "":
	   #second is unsigned byte
	   print("{}".format(struct.unpack('B', byte)[0]))
	#ring buffer can have one line of 0xFF. After that we might have more data
	if empty_reached == 0 and byte == '\xFF':
	   empty_reached = 1
	#temp cannot be 0xFF, end of the memory reached
	elif empty_reached == 1 and byte == '\xFF':
	   print("0xFF is detected twice. Exiting")
	   break
	byte = f.read(1)
print
