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
from subprocess import PIPE, Popen


#individual chip temperature offset to be added to the result
chip_temp_offset = -8
#config for test and programmer board
#CLK = 19
#DATA = 21
#RST = 15

#config for dedicated board
CLK = 21
DATA = 19
RST = 15

#voltage divider coefficients
ADC1_coeff = 10.63
ADC2_coeff = 10.63
ADC3_coeff = 10.63


def read_data():
	data = 0
	#if low attiny is not ready to send the new set of data yet
	while GPIO.input(DATA) != GPIO.LOW:
		pass
	for i in range(0, 16):
 		GPIO.output(CLK,GPIO.HIGH)
 		sleep(0.001)
 		data = (GPIO.input(DATA) << i) | data 
 		GPIO.output(CLK,GPIO.LOW)
 		sleep(0.001)
 	return data

def get_cpu_temperature():
    """get cpu temperature using vcgencmd"""
    process = Popen(['vcgencmd', 'measure_temp'], stdout=PIPE)
    output, _error = process.communicate()
    return float(output[output.index('=') + 1:output.rindex("'")])

raspi_temp = get_cpu_temperature()
	
GPIO.setmode(GPIO.BOARD)


#prevent warnings
GPIO.setwarnings(False)
#output
GPIO.setup(CLK, GPIO.OUT, initial=GPIO.HIGH)
#input
GPIO.setup(DATA, GPIO.IN, pull_up_down = GPIO.PUD_UP)
# reset
GPIO.setup(RST, GPIO.OUT, initial=GPIO.LOW)

#perform a reset
GPIO.output(RST, GPIO.LOW)
sleep(0.001)
GPIO.output(RST, GPIO.HIGH)
sleep(0.001)

data_adc1 = read_data() #read adc1 / pb2
data_adc2 = read_data() #read adc2 / pb4
data_adc3 = read_data() #read adc3 / pb3
data_vcc = read_data() #read vcc
data_temp = read_data() #read temp

vcc_ref= (1.1*1023)/data_vcc

now = datetime.datetime.now()
now.strftime("%d.%m.%Y %H:%M:%S")
print('{0} {1:2.3f}V {2:2.3f}V {3:2.3f}V {4:2.3f}V {5:+03d}degC {6:+03.0f}degC'.format(now.strftime('%d.%m.%Y %H:%M:%S'), 
	round(data_adc1*vcc_ref/1023*ADC1_coeff,2), round(data_adc2*vcc_ref/1023*ADC2_coeff,2), round(data_adc3*vcc_ref/1023*ADC3_coeff,2),
	round((1.1*1023)/data_vcc,2), data_temp+chip_temp_offset, round(raspi_temp)))


#exit and clean
GPIO.output(RST,GPIO.LOW)
sleep(0.001)
GPIO.output(RST,GPIO.HIGH)
GPIO.output(CLK,GPIO.LOW)

#change all pins to input, which is normally the default status
GPIO.cleanup()




