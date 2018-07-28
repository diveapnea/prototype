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

cb = CircularBuffer(all_bytes)


#cb.set_exit(True)
#while True:
#	print("{00:d}".format(struct.unpack('B', cb.next())[0])),


#find the start with at least three times 0xFF as the microprocessor does
#TODO: check if we start condition does not exist at all
while True:
	if cb.next() == '\xFF':
		if cb.next() == '\xFF':
			if cb.next() == '\xFF':
				break
print "Starting condition found at: ", cb.get_current_index()

#if looped once exit the script
cb.set_exit(True)

#exit the rest of the 7 records (3 were recorded as 0xFF
cb.next()
cb.next()
cb.next()
cb.next()

while True:
	byte = cb.next()	
	# day and month are unsigned byte
	print("{0:02d}.{1:02d}.2018".format(struct.unpack('B', byte)[0],struct.unpack('B', cb.next())[0])),
	   		
	byte = cb.next()
	#hour and minute are unsigned byte
	print("{0:02d}:{1:02d}".format(struct.unpack('B', byte)[0],struct.unpack('B', cb.next())[0])),
	
	byte = cb.next()
	#Voltage is unsigned byte. 15.9 is the resistor divider value
	print("{0:05.2f}V".format(struct.unpack('B', byte)[0]*1.1*15.9/255)),
	
	byte = cb.next()
	# Voltage is unsigned byte
	if (struct.unpack('B', byte)[0]) !=0:
		print("{0:.2f}V".format(255*1.1/struct.unpack('B', byte)[0])),
	else:
		print("0.00V"),
	
	byte = cb.next()
	#temp can be negative (signed byte)
	print("{0:+03d}degC".format(struct.unpack('b', byte)[0]-temp_offset))

print ""
