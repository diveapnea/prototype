
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


void initADC_10bit_ADC1()
{

  /* this function initialises the ADC 

  For more information, see table 17.5 "ADC Prescaler Selections" in 
  chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
  (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

  
  // 10-bit resolution
  // set ADLAR to 0 to disable left-shifting the result (bits ADC9 + ADC8 are in ADC[H/L] and 
  // bits ADC7..ADC0 are in ADC[H/L])
  // use uint16_t variable to read ADC (intead of ADCH or ADCL) 
  
  */

  ADMUX =
	    	(0 << ADLAR) |     // do not left shift result (for 10-bit values)
	    	(0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            //(0 << REFS2) |     // Sets ref. voltage to internal 1.1V, bit 2
            //(1 << REFS1) |     // Sets ref. voltage to internal 1.1V, bit 1   
			//(0 << REFS0) |     // Sets ref. voltage to internal 1.1V, bit 0
			//(0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            //(0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            //(1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            //(0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
            (0 << MUX3)  |     // use ADC1 for input (PB2), MUX bit 3
            (0 << MUX2)  |     // use ADC1 for input (PB2), MUX bit 2
            (0 << MUX1)  |     // use ADC1 for input (PB2), MUX bit 1
            (1 << MUX0);       // use ADC1 for input (PB2), MUX bit 0
            //(0 << MUX3)  |     // use ADC3 for input (PB3), MUX bit 3
            //(0 << MUX2)  |     // use ADC3 for input (PB3), MUX bit 2
            //(1 << MUX1)  |     // use ADC3 for input (PB3), MUX bit 1
            //(1 << MUX0);       // use ADC3 for input (PB3), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 128, bit 2 
            (1 << ADPS1) |     // set prescaler to 128, bit 1 
            (1 << ADPS0);      // set prescaler to 128, bit 0  
}

void initADC_10bit_ADC2()
{

  /* this function initialises the ADC 

  For more information, see table 17.5 "ADC Prescaler Selections" in 
  chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
  (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

  
  // 10-bit resolution
  // set ADLAR to 0 to disable left-shifting the result (bits ADC9 + ADC8 are in ADC[H/L] and 
  // bits ADC7..ADC0 are in ADC[H/L])
  // use uint16_t variable to read ADC (intead of ADCH or ADCL) 
  
  */

  ADMUX =
	    	(0 << ADLAR) |     // do not left shift result (for 10-bit values)
	    	(0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            //(0 << REFS2) |     // Sets ref. voltage to internal 1.1V, bit 2
            //(1 << REFS1) |     // Sets ref. voltage to internal 1.1V, bit 1   
			//(0 << REFS0) |     // Sets ref. voltage to internal 1.1V, bit 0
			(0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            (1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            (0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
            //(0 << MUX3)  |     // use ADC1 for input (PB2), MUX bit 3
            //(0 << MUX2)  |     // use ADC1 for input (PB2), MUX bit 2
            //(0 << MUX1)  |     // use ADC1 for input (PB2), MUX bit 1
            //(1 << MUX0);       // use ADC1 for input (PB2), MUX bit 0
            //(0 << MUX3)  |     // use ADC3 for input (PB3), MUX bit 3
            //(0 << MUX2)  |     // use ADC3 for input (PB3), MUX bit 2
            //(1 << MUX1)  |     // use ADC3 for input (PB3), MUX bit 1
            //(1 << MUX0);       // use ADC3 for input (PB3), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 128, bit 2 
            (1 << ADPS1) |     // set prescaler to 128, bit 1 
            (1 << ADPS0);      // set prescaler to 128, bit 0  
}

void initADC_10bit_ADC3()
{

  /* this function initialises the ADC 

  For more information, see table 17.5 "ADC Prescaler Selections" in 
  chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
  (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

  
  // 10-bit resolution
  // set ADLAR to 0 to disable left-shifting the result (bits ADC9 + ADC8 are in ADC[H/L] and 
  // bits ADC7..ADC0 are in ADC[H/L])
  // use uint16_t variable to read ADC (intead of ADCH or ADCL) 
  
  */

  ADMUX =
	    	(0 << ADLAR) |     // do not left shift result (for 10-bit values)
	    	(0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            //(0 << REFS2) |     // Sets ref. voltage to internal 1.1V, bit 2
            //(1 << REFS1) |     // Sets ref. voltage to internal 1.1V, bit 1   
			//(0 << REFS0) |     // Sets ref. voltage to internal 1.1V, bit 0
			//(0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            //(0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            //(1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            //(0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
            //(0 << MUX3)  |     // use ADC1 for input (PB2), MUX bit 3
            //(0 << MUX2)  |     // use ADC1 for input (PB2), MUX bit 2
            //(0 << MUX1)  |     // use ADC1 for input (PB2), MUX bit 1
            //(1 << MUX0);       // use ADC1 for input (PB2), MUX bit 0
            (0 << MUX3)  |     // use ADC3 for input (PB3), MUX bit 3
            (0 << MUX2)  |     // use ADC3 for input (PB3), MUX bit 2
            (1 << MUX1)  |     // use ADC3 for input (PB3), MUX bit 1
            (1 << MUX0);       // use ADC3 for input (PB3), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 128, bit 2 
            (1 << ADPS1) |     // set prescaler to 128, bit 1 
            (1 << ADPS0);      // set prescaler to 128, bit 0  
}



void initADC_temp()
{

  /* this function initialises the ADC to read the internal temperature sensor

  For more information, see table 17.5 "ADC Prescaler Selections" in 
  chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
  (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)
  */

  ADMUX =
	    	(0 << ADLAR) |     // do not left shift result (for 10-bit values)
            (0 << REFS2) |     // Sets ref. voltage to internal 1.1V, bit 2
            (1 << REFS1) |     // Sets ref. voltage to internal 1.1V, bit 1   
			(0 << REFS0) |     // Sets ref. voltage to internal 1.1V, bit 0
            (1 << MUX3)  |     // use ADC4 for input, MUX bit 3
            (1 << MUX2)  |     // use ADC4 for input, MUX bit 2
            (1 << MUX1)  |     // use ADC4 for input, MUX bit 1
            (1 << MUX0);       // use ADC4 for input, MUX bit 0

  ADCSRA = 
			(0 << ADATE) |     // disable auto trigger 
            (0 << ADIE)  |     // and disable interrupt
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 128, bit 2 
            (1 << ADPS1) |     // set prescaler to 128, bit 1 
            (1 << ADPS0);      // set prescaler to 128, bit 0  
}


void initADC_vcc(){

/*
Setting REFS[2:0] to 0bX00 will use VCC as the voltage reference, 
and setting MUX[3:0] to 0b1100 will use the internal bandgap voltage 
as the voltage to measure (see §17.13 from the datasheet). 
From there, the full range of 1023 will tell you what VCC is in relation to the bandgap voltage. 
So take (1.1*10*1023=) 11253 (or an appropriately scaled equivalent) and divide it by the 
measured value in order to get the approximate value of VCC in tenths of volts.


Use VCC as VREF and measure the Bandgap 1.1V voltage.
ADCW = 1023 * 1.1 / VCC
rearranging
VCC = 1.1*1023 / ADCW

*/

  ADMUX =
	    	(0 << ADLAR) |     // do not left shift result (for 10-bit values)
	    	(0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            //(0 << REFS2) |     // Sets ref. voltage to internal 1.1V, bit 2
            //(1 << REFS1) |     // Sets ref. voltage to internal 1.1V, bit 1   
			//(0 << REFS0) |     // Sets ref. voltage to internal 1.1V, bit 0
			//(0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            //(0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            //(1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            //(0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
            //(0 << MUX3)  |     // use ADC1 for input (PB2), MUX bit 3
            //(0 << MUX2)  |     // use ADC1 for input (PB2), MUX bit 2
            //(0 << MUX1)  |     // use ADC1 for input (PB2), MUX bit 1
            //(1 << MUX0);       // use ADC1 for input (PB2), MUX bit 0
            (1 << MUX3)  |     // use V_bandgap for input,  MUX bit 3
            (1 << MUX2)  |     // use V_bandgap for input,  MUX bit 2
            (0 << MUX1)  |     // use V_bandgap for input,  MUX bit 1
            (0 << MUX0);       // use V_bandgap for input,  MUX bit 0
            //(0 << MUX3)  |     // use ADC1 for input (PB3), MUX bit 3
            //(0 << MUX2)  |     // use ADC1 for input (PB3), MUX bit 2
            //(1 << MUX1)  |     // use ADC1 for input (PB3), MUX bit 1
            //(1 << MUX0);       // use ADC1 for input (PB3), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 128, bit 2 
            (1 << ADPS1) |     // set prescaler to 128, bit 1 
            (1 << ADPS0);      // set prescaler to 128, bit 0  

}

uint16_t read_10bit_ADC(){

   uint16_t raw_adc;


    ADCSRA |= (1 << ADSC);         // start ADC measurement
    while (ADCSRA & (1 << ADSC) ); // wait till conversion complete 

	//very important to read the registers like this. Otherwise it will block and not work
	uint8_t low_byte=ADCL; //reading this blocks the conversion
	uint8_t high_byte=ADCH; //now conversion can continue
	
    // for 10-bit resolution:
    raw_adc = ((high_byte<<8) | low_byte);   // add lobyte and hibyte

    return raw_adc; 
}
  




int8_t read_temp(){

// take 100x 10-bit samples and calculate a rolling average of the last 15 samples
  
 float temp_degC=0.0;        // internal temp
 int sample_loop;

 for (sample_loop=100; sample_loop > 0 ; sample_loop --){

   temp_degC = temp_degC + ((read_10bit_ADC() - temp_degC) / 15);  // integrated last 15-sample rolling average
  }
  return (int8_t) (temp_degC-273); //return value in degC 
}

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
	 //PORTB |= (1<<pin); //set the pull-up pin
	}  
else 
	return;
}




///////////////////////////////////////
//void send_data()
///////////////////////////////////////
// Uncomment if you want to use. Commented to make the hex file as small as possible
void send_data(uint16_t data){
//sends data via SPI (own protocol)
//output MISO pin6 --> GPIO09 board pin21 on raspberry
//clock in MOSI input 5 --> GPIO10 board pin19 on raspberry

pinMode(PB0, INPUT); //pin5 --> this is the clock from raspi
pinMode(PB1, OUTPUT); //pin6 --> SPI output of attiny (MISO - Master Input, Slave Output)


digitalWrite(PB1, LOW); //if low we are ready to send
  
uint8_t bit_count=0;

 for(bit_count=0;bit_count<16;bit_count++){
  //wait for clock to get high
  while(!digitalRead(PB0)){
	//wait for the clock
  }
  //send the first bit LSB
  digitalWrite(PB1, (data >> bit_count) & 1);
  
  //wait for clock to get low
  while(digitalRead(PB0)){
	//wait for the clock
  }
  
 }

digitalWrite(PB1, HIGH); //if high we are not ready to send

}





/*
			PCINT5, Reset  1 *      8  VCC 5 Volts
PCINT3, Analog In 3, PB3   2        7  PB2, Analog In 1, I2C SCL, SPI SCK, PCINT2
PCINT4, Analog In 2, PB4   3        6  PB1, PWM, SPI MISO, PCINT1
					Ground 4        5  PB0, PWM, I2C SDA, SPI MOSI, PCINT0
*/

int main() {


//to send data to raspberry (start sudo python get_data.py for voltage and get_data2.py for raw data on raspberry)

for(;;){
	//VCC ref voltage is set (normally 3.3V if used with raspberry pi)
	initADC_10bit_ADC1(); //PB2
	//throw one set to have clean readings
	read_10bit_ADC();
	//send data waits until the data is completely read. so no need for delay
	send_data(read_10bit_ADC());

	//VCC ref voltage is set (normally 3.3V if used with raspberry pi)
	initADC_10bit_ADC2(); //PB4
	//throw one set to have clean readings
	read_10bit_ADC();
	//send data waits until the data is completely read. so no need for delay
	send_data(read_10bit_ADC());

	//VCC ref voltage is set (normally 3.3V if used with raspberry pi)
	initADC_10bit_ADC3(); //PB3
	//throw one set to have clean readings
	read_10bit_ADC();
	//send data waits until the data is completely read. so no need for delay
	send_data(read_10bit_ADC());

	initADC_vcc();
	//throw one set to have clean readings
	read_10bit_ADC();
	//send data waits until the data is completely read. so no need for delay
	send_data(read_10bit_ADC());
	
	initADC_temp();
	//send data waits until the data is completely read. so no need for delay
	send_data(read_temp());	
				
 }


return(0);
}
