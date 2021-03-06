
MCU		=attiny85
CLOCK		=8000000
AVRDUDEMCU	=t85
CC		=/usr/bin/avr-g++
CFLAGS		=-g -Os -Wall -mcall-prologues -DF_CPU=$(CLOCK) -mmcu=$(MCU)
OBJ2HEX		=/usr/bin/avr-objcopy
AVRDUDE		=/usr/bin/avrdude
TARGET		=genetic
EEPROM		=eeprom.hex

all :
	$(CC) $(CFLAGS) -c $(TARGET).cpp -o $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).o
	#$(CC) $(CFLAGS) $(TARGET).cpp -o $(TARGET)
	#$(OBJ2HEX) -R .eeprom -O ihex $(TARGET) $(TARGET).hex
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	avr-size --format=avr --mcu=$(MCU) $(TARGET).elf

	rm -f $(TARGET)

install : all
	sudo gpio -g mode 9 alt0
	sudo gpio -g mode 10 alt0
	sudo gpio -g mode 11 alt0
	sudo gpio -g mode 22 out
	sudo gpio -g write 22 0
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -V -P /dev/spidev0.0 -c linuxspi -b 10000 -U flash:w:$(TARGET).hex
	sudo gpio -g write 22 1

noreset : all
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -V -P /dev/spidev0.0 -c linuxspi -b 10000 -U flash:w:$(TARGET).hex

fuse :
	sudo gpio -g mode 9 alt0
	sudo gpio -g mode 10 alt0
	sudo gpio -g mode 11 alt0
	sudo gpio -g mode 22 out
	sudo gpio -g write 22 0
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -P /dev/spidev0.0 -c linuxspi -b 10000 -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
	sudo gpio -g write 22 1

clean :
	rm -f *.hex *.obj *.o *.elf

eeprom :
	sudo gpio -g mode 9 alt0
	sudo gpio -g mode 10 alt0
	sudo gpio -g mode 11 alt0
	sudo gpio -g mode 22 out
	sudo gpio -g write 22 0
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -V -P /dev/spidev0.0 -c linuxspi -b 10000 -U eeprom:r:$(EEPROM):r
	sudo gpio -g write 22 1
	sudo chown pi:pi $(EEPROM)
	python calc_score.py $(EEPROM)
	#hexdump -C $(EEPROM)
	#od -An -tu1 $(EEPROM) 

