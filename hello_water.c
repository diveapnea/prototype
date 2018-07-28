
//en onemli
//https://www.heise.de/developer/artikel/ATtiny-Winzlinge-ganz-gross-3329007.html

//programmer wiring
//http://www.instructables.com/id/Programming-the-ATtiny85-from-Raspberry-Pi/step2/Electrical-Connections/

//wireless 433MHz
//https://arduinodiy.wordpress.com/2015/03/29/sending-wireless-data-from-one-attiny85-to-another/

// avr-gcc -Wall -Os -DF_CPU=6000000 -mmcu=attiny85 -c hello.c -o hello.o
// avr-gcc -Wall -Os -DF_CPU=6000000 -mmcu=attiny85 -o hello.elf hello.o
//avr-objcopy -j .text -j .data -O ihex hello.elf hello.hex
//avr-size --format=avr --mcu=attiny85 hello.elf
/**
 *  
 * Blinking LED ATTiny85 "hello world"
 *
 * Following the tutorial at:
 * http://www.instructables.com/id/Honey-I-Shrunk-the-Arduino-Moving-from-Arduino-t/?ALLSTEPS
 * following page for the ADC conversion
 * https://www.marcelpost.com/wiki/index.php/ATtiny85_ADC 
 */

/**
* https://teslaui.wordpress.com/2013/03/26/attiny85-port-registers/ 
PORT is for outputting data I/O.
PIN is for reading in data I/O.
* PORTB: Depends on the configuration of the DDRB. 
*   If DDRB is an input, PORTB sets the internal Pull-up resistors (0=off; 1=on). 
*   If DDRB is an output, PORTB sets high/low outputs (0=low; 1=high). 
*   Single ports are addressed by PORTB0-PORTB5.
* DDRB: Selects the direction of a pin (0=Input; 1=Output). 
*   Single pins are addressed by DDB0-DDB5.
* PINB: A logical 1 toggles the according bit in PORTB. 
*   Single port input pins are addressed by PIN0-PIN5.
*/

#include <avr/io.h>
// F_CPU frequency to be defined at command line
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h> //Needed to enable/disable watch dog timer
#include <avr/interrupt.h>      // library for interrupts handling
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/power.h>  //power saving functions
#include "TinyWireM.h"

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0


#define DS3231_I2C_ADDRESS 0x68

uint8_t emergency=0;

/* functions in power.h
power_adc_enable()   
power_adc_disable()     

power_timer0_enable()  
power_timer0_disable()  

power_timer1_enable()  
power_timer1_disable()  

//Universal Serial Interface
power_usi_enable()     
power_usi_disable()    

power_all_enable()     
power_all_disable() 
*/


uint8_t digitalRead(uint8_t pin){
 if( (PINB & (1<<pin)) ) 
 	return HIGH;
 else return LOW;
}

void digitalWrite(uint8_t pin, uint8_t val){
 if (val==HIGH)    
    PORTB |= (1<<pin);  
else if (val==LOW)
	PORTB &= ~(1<<pin); 
else 
	return;
}

void pinMode(uint8_t pin, uint8_t mode){
 if (mode==OUTPUT)    
    DDRB |= (1<<pin);  
else if (mode==INPUT){
	 DDRB &= ~(1<<pin); 
	 //PORTB |= (1<<pin); //set the pull-up pin OR set the same input pin to HIGH with digitalWrite
	}  
else 
	return;
}


void system_sleep() {
 
 //think about pins states for power consumption reduction
  
 ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
 power_all_disable(); //shuts down everything
  

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  
  sei(); //enable interrupts so that we can wake-up
  sleep_mode();                        // System actually sleeps here
  cli(); //disable interrupts so that we are not interrupted doing work
  sleep_disable();                     // System continues execution here when watchdog timed out 
  
  
   power_all_enable(); //start everything

}



//runs when pin interrupt is triggered
ISR(PCINT0_vect)
{
   emergency=1; //emergency situation
}


/*
			PCINT5, Reset  1 *      8  VCC 5 Volts
PCINT3, Analog In 3, PB3   2        7  PB2, Analog In 1, I2C SCL, SPI SCK, PCINT2
PCINT4, Analog In 2, PB4   3        6  PB1, PWM, SPI MISO, PCINT1
					Ground 4        5  PB0, PWM, I2C SDA, SPI MOSI, PCINT0
*/

int main() {


//WATER DETECTOR

// Ensure no floating during sleep
DDRB = 0xFF; // PORTB is output, all pins
PORTB = 0x00; // Make pins low


/*
  8 MHz cpu
  TCCR1 = B00000001; prevents Timer/Counter-1 from using pin OC1A, prescaler = 1 (=PCK)
  TCCR1 = B00000001; prevents Timer/Counter-1 from using pin OC1A, prescaler = PCK/4
  GTCCR = B01100000; enables pwm on OC1B
  GTCCR = B01010000; enables pwm on OC1B and inverse(OC1B)
  OCR1C = 140; f(PWM output) = 8 MHz/prescaler/(OCR1C+1) this may require some fine-tuning
  OCR1B = 70; OCR1C/2 for 50% duty cycle
*/

  pinMode(PB4, OUTPUT);
  pinMode(PB3, OUTPUT);
  TCCR1 = 0b00000011;
  //GTCCR = 0b01010000; //enable PWM outputs on PB4 and PB3
  GTCCR = 0b01000000; //disable PWM outputs for now
  OCR1C = 100;//140;
  OCR1B = 70;


//set interrupt pin as input
pinMode(PB0, INPUT);
digitalWrite(PB0, HIGH); //set the pull-up pin


//general interrupt mask register
//set the pin interrupt
GIMSK |= (1<<PCIE);
//set interrupt for PCINT0 --> PB0 --> Pin5
PCMSK |= (1<< PCINT0);

system_sleep();

sei(); //also with system_sleep this is required right here

while(1){
	if(emergency){
		GTCCR = 0b01010000; //enable the alarm
	}

_delay_ms(50);
OCR1C = 90; //set a frequency (2.2kHz)
_delay_ms(50);
OCR1C = 100; //set another frequency (2kHz)

}

return(0);
}
