import struct
import sys
#import pandas as pd

if len(sys.argv) !=2:
   print "Wrong number of arguments. We need one epprom hex to decode"
   exit(1)


class CircularBuffer(object):
#    def __init__(self, size):
#        """Initialization"""
#        self.index = 0
#        self.size = size
#        self._data = []

	def __init__ (self, mylist):
		"""Init with a known list"""
		self.index = 0
		self.marked_index = 0
		self.size = len(mylist)
		self._data = list(mylist)
		self._exit = False
		
	def append(self, value):
		"""Append an element"""
		if len(self._data) == self.size:
			self._data[self.index] = value
		else:
			self._data.append(value)
		self.index = (self.index + 1) % self.size

	def next(self):
		"""Get element by index, relative to the current index"""
		self.index = (self.index+1) % self.size
		if self.index == self.marked_index and self._exit == True:
			print "End of ring buffer. Exiting."
			quit() 
		return(self._data[self.index])
        
	def set_exit(self, condition):
		"""Set exit condition when back at the current index"""
		self._exit = condition
		self.marked_index = self.index
        
	def get_current_index(self):
		"""Get current index"""
		return(self.index)



#temp_offset for this specific attiny85
#temp_offset = 0 
temp_offset = 6 
#reached FF area?
start_found = False
print "Current temperature offset:", temp_offset, "degC"

#df = pd.DataFrame(columns = ["Date", "Time", "Voltage", "Vcc", "Temp"])
#df.loc[len(df)] = (["0.0","0:0",0,0,0])
#df.iloc[[0],[0]] = 1


#read all bytes in the file
with open(sys.argv[1], "rb") as f:
	all_bytes=f.read()

all_bytes=all_bytes[:510] #see the source code of the avr. We use only the 510 bytes out of 512

cb = CircularBuffer(all_bytes)


#cb.set_exit(True)
#while True:
#	print("{00:d}".format(struct.unpack('B', cb.next())[0])),


#find the start with at least three times 0xFF as the microprocessor does
#TODO: check if we start condition does not exist at all
while True:
	
	if cb.next() == '\xFF':
		#if looped once exit the script
		cb.set_exit(True)
		if cb.next() == '\xFF':
			if cb.next() == '\xFF':
				break

#exit the rest of the 6 records (3 were recorded as 0xFF
cb.next()
cb.next()
cb.next()

print "After starting condition, first data set expected at: ", cb.get_current_index()

line_number=1

while True:
	
	byte = cb.next()
	
	while byte == '\xFF': #empty set of data
			cb.next()
			cb.next()
			cb.next()
			cb.next()
			cb.next()
			byte = cb.next()
			
	# day and month are unsigned byte
	print("{0:03d}: {1:02d}.{2:02d}.2018".format(line_number, struct.unpack('B', byte)[0],struct.unpack('B', cb.next())[0])),
	   			
	byte = cb.next()
	#Min Voltage is unsigned byte. 15.9 is the resistor divider value
	print("V_ext_min: {0:05.2f}V".format(struct.unpack('B', byte)[0]*1.1*15.9/255)),
	
	byte = cb.next()
	#Min VCC is unsigned byte
	if (struct.unpack('B', byte)[0]) !=0:
		print("Vcc_min: {0:.2f}V".format(255*1.1/struct.unpack('B', byte)[0])),
	else:
		print("Vcc_min: 0.00V"),
	
	byte = cb.next()
	#Max temp can be negative (signed byte)
	print("T_max: {0:+03d}degC".format(struct.unpack('b', byte)[0]-temp_offset)),

	byte = cb.next()
	#Min temp can be negative (signed byte)
	print("T_min: {0:+03d}degC".format(struct.unpack('b', byte)[0]-temp_offset))

	line_number = line_number+6

print ""
