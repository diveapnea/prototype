import RPi.GPIO as GPIO
from time import sleep
import sys

GPIO.setmode(GPIO.BOARD)
#output
GPIO.setup(19, GPIO.OUT, initial=GPIO.LOW)
#input
GPIO.setup(21, GPIO.IN, pull_up_down = GPIO.PUD_UP)
# reset
GPIO.setup(15, GPIO.OUT, initial=GPIO.HIGH)

#perform a reset
GPIO.output(15, GPIO.LOW)
sleep(0.3)
GPIO.output(15, GPIO.HIGH)
sleep(0.3)

adr=0

try:
 while True:
  data = 0
  for i in range(0, 16):
   GPIO.output(19,GPIO.HIGH)
   sleep(0.01)
   #sleep(0.001) #that works too
   data = (GPIO.input(21) << i) | data 
   GPIO.output(19,GPIO.LOW)
   sleep(0.01)
   #sleep(0.001) #that works too
  print "Adr {}: {}".format(adr, data)  
  adr += 1
  #print float(data*3.3/1023)
  #print data
  sys.stdout.flush()
except KeyboardInterrupt:
    pass

print "\nExiting"
#reset again
GPIO.output(15,GPIO.LOW)
sleep(0.3)
GPIO.output(15,GPIO.HIGH)
sleep(0.3)
GPIO.output(19,GPIO.LOW)
GPIO.cleanup()
