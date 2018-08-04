import serial
import re
import matplotlib.pyplot as plt
import collections
import decimal

 
def init_plot():
	#turn on interactive plotting
	plt.ion()
	plt.figure()
	plt.title("Reading", fontsize=15)
	plt.xlabel("time", fontsize=13)
	plt.ylabel("Voltage and Current", fontsize=13)
	plt.autoscale(enable=True, axis='y')
	plt.grid(True)

	
def continuous_plot(array_x, array_v, array_a):
	global text_var_v, text_var_a, line_v, line_a, text_max_v, text_min_v, text_max_a, text_min_a
	
	line_v.pop(0).remove()
	line_a.pop(0).remove()
	
	text_max_v.remove() #remove the old text
	text_min_v.remove() #remove the old text
	text_max_a.remove() #remove the old text
	text_min_a.remove() #remove the old text
	text_var_v.remove() #remove the old text
	text_var_a.remove() #remove the old text
	
	line_v = plt.plot(array_x, array_v, '-b.')
	line_a = plt.plot(array_x, array_a, '-r.')
	plt.xlim(xmin=min(array_x), xmax=max(array_x))
		
	text_var_v = plt.text(array_x[-1], array_v[-1], str(array_v[-1])+'V')
	text_var_a = plt.text(array_x[-1], array_a[-1], str(array_a[-1])+'A')
	
	#find and mark the maximum value
	ymax = max(i for i in array_v if i is not None)
	xpos = list(array_v).index(ymax)
	xmax = array_x[xpos]
	text_max_v = plt.text(xmax, ymax, str(ymax)+'V')
	
	#find and mark the minimum value
	ymin = min(i for i in array_v if i is not None)
	xpos = list(array_v).index(ymin)
	xmin = array_x[xpos]
	text_min_v = plt.text(xmin, ymin, str(ymin)+'V')
	
	#find and mark the maximum value
	ymax = max(i for i in array_a if i is not None)
	xpos = list(array_a).index(ymax)
	xmax = array_x[xpos]
	text_max_a = plt.text(xmax, ymax, str(ymax)+'A')
	
	#find and mark the minimum value
	ymin = min(i for i in array_a if i is not None)
	xpos = list(array_a).index(ymin)
	xmin = array_x[xpos]
	text_min_a = plt.text(xmin, ymin, str(ymin)+'A')
	
	plt.draw()

ser = serial.Serial('/dev/cu.wchusbserialfa130', 1000000)

init_plot()

size = 10000
array_v = collections.deque([None] * size, maxlen=size)
array_a = collections.deque([None] * size, maxlen=size)
array_x = collections.deque([0] * size, maxlen=size)
text_var_v = plt.text(0,0,'') #dummy
text_var_a = plt.text(0,0,'') #dummy
text_max_v = plt.text(0,0,'') #dummy
text_min_v = plt.text(0,0,'') #dummy
text_max_a = plt.text(0,0,'') #dummy
text_min_a = plt.text(0,0,'') #dummy
line_v = plt.plot(0,0) #dummy
line_a = plt.plot(0,0) #dummy

try:
	while True:
		for i in xrange(size):
			#sometimes data drops. Be tolerant
			try:
				value = decimal.Decimal(ser.readline())
				if value.as_tuple().exponent <-1: #this must be the voltage
					array_v.append(value) #fixed array size to prevent speed degradation
				else: 
					array_a.append(value) #fixed array size to prevent speed degradation
					array_x.append(array_x[-1]+1) #fixed array size to prevent speed degradation
			except Exception:
				pass
		continuous_plot(array_x,array_v,array_a)
except KeyboardInterrupt:
	pass
    

    
ser.close() # Only executes once the loop exits
print 'Exiting...'
