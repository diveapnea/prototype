import RPi.GPIO as GPIO
import time
import os

GPIO.setmode(GPIO.BCM)

GPIO.setup(27, GPIO.IN, pull_up_down=GPIO.PUD_UP)

while True:
	input_state = GPIO.input(27)
	if input_state == False:
		time.sleep(1)
		input_state = GPIO.input(27)
		if input_state == False:
			os.system("sudo shutdown -h now")
			break
		else:
			os.system("omxplayer /home/pi/Development/Football-crowd.mp3")
	time.sleep(0.2)
