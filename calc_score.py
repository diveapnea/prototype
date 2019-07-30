import struct
import sys
#import pandas as pd

if len(sys.argv) !=2:
   print "Wrong number of arguments. We need one epprom hex to decode"
   exit(1)


#read first 256 bytes in the file

with open(sys.argv[1], "rb") as f:
	all_bytes=f.read()[0:256]

score = 0

for byte in all_bytes:
	score += struct.unpack('B', byte)[0]

print "Score ", score
