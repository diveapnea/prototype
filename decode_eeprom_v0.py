import struct
import sys
#import pandas as pd

if len(sys.argv) !=2:
   print "Wrong number of arguments. We need one epprom hex to decode"
   exit(1)

#temp_offset for this specific attiny85
#temp_offset = 0 
temp_offset = 6 
#reached FF area?

print "Current temperature offset:", temp_offset, "degC"

#df = pd.DataFrame(columns = ["Date", "Time", "Voltage", "Vcc", "Temp"])
#df.loc[len(df)] = (["0.0","0:0",0,0,0])
#df.iloc[[0],[0]] = 1


#read all bytes in the file
with open(sys.argv[1], "rb") as f:
	all_bytes=f.read()

all_bytes_iter = iter(all_bytes)

for byte in all_bytes_iter:
	#ring buffer can have one line of 0xFF. After that we might have more data
	if byte == '\xFF':
		#throw the empty line away
		byte = next(all_bytes_iter)
		byte = next(all_bytes_iter)
		byte = next(all_bytes_iter)
		byte = next(all_bytes_iter)
		byte = next(all_bytes_iter)
		byte = next(all_bytes_iter)
		byte = next(all_bytes_iter)
		if byte == '\xFF':
			print
			print("0xFF is detected twice. Exiting")
			break
	
	# day and month are unsigned byte
	print("{0:02d}.{1:02d}.2017".format(struct.unpack('B', byte)[0],struct.unpack('B', next(all_bytes_iter))[0])),
	   		
	byte = next(all_bytes_iter)
	#hour and minute are unsigned byte
	print("{0:02d}:{1:02d}".format(struct.unpack('B', byte)[0],struct.unpack('B', next(all_bytes_iter))[0])),
	
	byte = next(all_bytes_iter)
	#Voltage is unsigned byte. 15.9 is the resistor divider value
	print("{0:05.2f}V".format(struct.unpack('B', byte)[0]*1.1*15.9/255)),
	
	byte = next(all_bytes_iter)
	# Voltage is unsigned byte
	if (struct.unpack('B', byte)[0]) !=0:
		print("{0:.2f}V".format(255*1.1/struct.unpack('B', byte)[0])),
	else:
		print("0.00V"),
	
	byte = next(all_bytes_iter)
	#temp can be negative (signed byte)
	print("{0:+03d}degC".format(struct.unpack('b', byte)[0]-temp_offset))

print ""
