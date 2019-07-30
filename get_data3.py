#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
Created on 26.07.2019

@author: Ugur Akin
'''

import RPi.GPIO as GPIO
from time import sleep
import sys
import datetime

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

try:
 while True:
  data = 0
  for i in range(0, 16):
   GPIO.output(19,GPIO.HIGH)
   sleep(0.01)
   data = (GPIO.input(21) << i) | data 
   GPIO.output(19,GPIO.LOW)
   sleep(0.01)
  now = datetime.datetime.now()
  now.strftime("%d.%m.%Y %H:%M:%S")
  print('{0} {1:.3f}V'.format(now.strftime('%d.%m.%Y %H:%M:%S'), round(data*3.3/1023,2)))
  #print float(data*3.3/1024)
  #print data
  sys.stdout.flush()
except KeyboardInterrupt:
    pass

print "\nExiting"
GPIO.output(15,GPIO.LOW)
sleep(0.1)
GPIO.output(15,GPIO.HIGH)
GPIO.output(19,GPIO.LOW)
GPIO.cleanup()
