
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
// has to be a multiple of the max number of records. Otherwise cells shift with every loop to cell 0 in buffer
//max buffer size is physically given as 512
#define MAX_SIZE 510 //6 records x 83 = 510 //7 records x 73 = 511
//min number of sensor readings before accepting a day as suitable for min max recording
#define MIN_READING 20

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define DS3231_I2C_ADDRESS 0x68

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

/* functions in eeprom.h
#define eeprom_read_byte      _EEPROM_CONCAT2 (__eerd_byte, _EEPROM_SUFFIX)
#define eeprom_read_word      _EEPROM_CONCAT2 (__eerd_word, _EEPROM_SUFFIX)
#define eeprom_read_dword     _EEPROM_CONCAT2 (__eerd_dword, _EEPROM_SUFFIX)
#define eeprom_read_float     _EEPROM_CONCAT2 (__eerd_float, _EEPROM_SUFFIX)
#define eeprom_read_block     _EEPROM_CONCAT2 (__eerd_block, _EEPROM_SUFFIX)

#define eeprom_write_byte     _EEPROM_CONCAT2 (__eewr_byte, _EEPROM_SUFFIX)
#define eeprom_write_word     _EEPROM_CONCAT2 (__eewr_word, _EEPROM_SUFFIX)
#define eeprom_write_dword    _EEPROM_CONCAT2 (__eewr_dword, _EEPROM_SUFFIX)
#define eeprom_write_float    _EEPROM_CONCAT2 (__eewr_float, _EEPROM_SUFFIX)
#define eeprom_write_block    _EEPROM_CONCAT2 (__eewr_block, _EEPROM_SUFFIX)

//update writes only if the existing value is different than the one you intend to write
#define eeprom_update_byte    _EEPROM_CONCAT2 (__eeupd_byte, _EEPROM_SUFFIX)
#define eeprom_update_word    _EEPROM_CONCAT2 (__eeupd_word, _EEPROM_SUFFIX)
#define eeprom_update_dword   _EEPROM_CONCAT2 (__eeupd_dword, _EEPROM_SUFFIX)
#define eeprom_update_float   _EEPROM_CONCAT2 (__eeupd_float, _EEPROM_SUFFIX)
#define eeprom_update_block   _EEPROM_CONCAT2 (__eeupd_block, _EEPROM_SUFFIX)
eeprom_is_ready()
eeprom_busy_wait()
*/


//global watchdog counter
volatile uint16_t watchdog_counter = 0;

//global pin interrupt flag
//volatile uint16_t int_flag = 0;

//global eeprom address 0-511
uint16_t eeprom_adr = 0;

//global min temp
int8_t min_temp = INT8_MAX; //0x7f;

//global max temp
int8_t max_temp = INT8_MIN; //(-INT8_MAX - 1)

//global min vcc
uint8_t min_vcc = UINT8_MAX;

//global min voltage
uint8_t min_v = UINT8_MAX;

//global sensor reasing per day
uint8_t reading = 0;

//global day variable
uint8_t recording_day = 0;

//global day variable
uint8_t recording_month = 0;


//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {
  watchdog_counter++;
}

//This generator is free to use, but is not suitable for cryptography due to its short period(by //cryptographic standards) and simple construction. No attempt was made to make this generator
// suitable for cryptographic use.
//
//Due to the use of a constant counter, the generator should be resistant to latching up.
//A significant performance gain is had in that the x variable is nly ever incremented.
//
//Only 4 bytes of ram are needed for the internal state, and generating a byte requires 3 XORs , //2 ADDs, one bit shift right , and one increment. Difficult or slow operations like multiply, etc
//were avoided for maximum speed on ultra low power devices.

uint8_t a=0,b=0,c=0,x=0;

void init_rng(uint8_t s1, uint8_t s2, uint8_t s3) //Can also be used to seed the rng with more entropy during use.
{
//XOR new entropy into key state
a ^=s1;
b ^=s2;
c ^=s3;

x++;
a = (a^c^x);
b = (b+a);
c = c+((b>>1)^a);
}

uint8_t getRandom()
{
x++;               //x is incremented every round and is not affected by any other variable
a = (a^c^x);       //note the mix of addition and XOR
b = (b+a);         //And the use of very few instructions
c = c+((b>>1)^a);  //the right shift is to ensure that high-order bits from b can affect  
return(c);         //low order bits of other variables
}



//runs when pin interrupt is triggered
//ISR(PCINT0_vect)
//{
//change the value of a volatile variable to trigger a function
/*
1) Be as short as possible.  
2) Don’t use delays in the ISR.
3)  Use volatile global variables if you’re going to change them in the ISR.
*/
//int_flag=1;
//}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {
 if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings
 
 uint8_t bb = timerPrescaler & 7; 
 if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary
 
 //This order of commands is important and cannot be combined
 MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
 WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
 WDTCR = bb; //Set new watchdog timeout value
 WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

void initADC_10bit()
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
	    	//(0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            //(0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << REFS2) |     // Sets ref. voltage to internal 1.1V, bit 2
            (1 << REFS1) |     // Sets ref. voltage to internal 1.1V, bit 1   
			(0 << REFS0) |     // Sets ref. voltage to internal 1.1V, bit 0
			//(0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            //(0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            //(1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            //(0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
            //(0 << MUX3)  |     // use ADC1 for input (PB2), MUX bit 3
            //(0 << MUX2)  |     // use ADC1 for input (PB2), MUX bit 2
            //(0 << MUX1)  |     // use ADC1 for input (PB2), MUX bit 1
            //(1 << MUX0);       // use ADC1 for input (PB2), MUX bit 0
            (0 << MUX3)  |     // use ADC1 for input (PB3), MUX bit 3
            (0 << MUX2)  |     // use ADC1 for input (PB3), MUX bit 2
            (1 << MUX1)  |     // use ADC1 for input (PB3), MUX bit 1
            (1 << MUX0);       // use ADC1 for input (PB3), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 128, bit 2 
            (1 << ADPS1) |     // set prescaler to 128, bit 1 
            (1 << ADPS0);      // set prescaler to 128, bit 0  
}


///////////////////////////////////////
//void initADC_8bit()
///////////////////////////////////////
//void initADC_8bit()
//{
  /* this function initialises the ADC 

        ADC Prescaler Notes:
	--------------------

	   ADC Prescaler needs to be set so that the ADC input frequency is between 50 - 200kHz.
  
           For more information, see table 17.5 "ADC Prescaler Selections" in 
           chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
          (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

           Valid prescaler values for various clock speeds
	
	     Clock   Available prescaler values
           ---------------------------------------
             1 MHz   8 (125kHz), 16 (62.5kHz)
             4 MHz   32 (125kHz), 64 (62.5kHz)
             8 MHz   64 (125kHz), 128 (62.5kHz)
            16 MHz   128 (125kHz)

           Below example set prescaler to 128 for mcu running at 8MHz
           (check the datasheet for the proper bit values to set the prescaler)
  */

  // 8-bit resolution
  // set ADLAR to 1 to enable the Left-shift result (only bits ADC9..ADC2 are available)
  // then, only reading ADCH is sufficient for 8-bit results (256 values)
/*
  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            (1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            (0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 64, bit 2 
            (1 << ADPS1) |     // set prescaler to 64, bit 1 
            (0 << ADPS0);      // set prescaler to 64, bit 0  
}
*/

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
  
///////////////////////////////////////
//uint8_t read_8bit_ADC()
///////////////////////////////////////  
/*
uint8_t read_8bit_ADC(){

    ADCSRA |= (1 << ADSC);         // start ADC measurement
    while (ADCSRA & (1 << ADSC) ); // wait till conversion complete 

    return ADCH;
}
*/

///////////////////////////////////////
//float read_rolling_avarage()
///////////////////////////////////////
/*
float read_rolling_avarage(){
// take 100x 10-bit samples and calculate a rolling average of the last 15 samples
  
 float voltage_fl=0.0;        // real battery voltage (0-5V) with decimals
 float adc_step=1.1/1023;  // (0-1.1V over 1024 values)
 int sample_loop;

 for (sample_loop=100; sample_loop > 0 ; sample_loop --){

   voltage_fl = voltage_fl + (((read_10bit_ADC() * adc_step) - voltage_fl) / 15);  // integrated last 15-sample rolling average
  }
  return voltage_fl;
}
*/

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
//void blink_led()
///////////////////////////////////////
/*
void blink_led(){
   //and set PORTB4 as output for led blinking
 	pinMode(PB4, OUTPUT); //pin3
   for(;;){
    //start blinking
    digitalWrite(PB4,LOW); //pin3
	_delay_ms(100); // wait some time
    digitalWrite(PB4,HIGH); //pin3
	_delay_ms(100); // wait some time
   }  
}*/

void blink_led_x_times(uint8_t x){
   //and set PORTB4 as output for led blinking
 	pinMode(PB4, OUTPUT); //pin3
 	
 	uint8_t dummy_GTCCR = GTCCR; //backup
 	uint8_t dummy_TCCR1 = TCCR1; //backup
  //PWM configuration
  //be careful before changing timer pre-scalars since other functions like _delay_ms relay on them
  // Configure counter/timer0 for fast PWM on PB0 and PB1
  //TCCR0A = 3<<COM0A0 | 3<<COM0B0 | 3<<WGM00;
  //TCCR0B = 0<<WGM02 | 3<<CS00; // Optional; already set
  // Configure counter/timer1 for fast PWM on PB4
  OCR1B=255; //make sure that output does not go on high during init
  GTCCR = 1<<PWM1B | 3<<COM1B0;
  TCCR1 = 3<<COM1A0 | 7<<CS10;
 	
 	//PWM configuration	
	//Output PB0: OCR0A or OCR1A.
	//Output PB1: OCR0B or OCR1A.
	//Output PB2: none.
	//Output PB3: OCR1B.
	//Output PB4: OCR1B.
 	
  uint8_t i;    
  int j; //this one has to be signed
  for(i=0;i<x;i++){
  //start blinking smoothly
  	//go on (inverted mode)
	for (j=255; j >= 0; j--) {
		OCR1B=j;
		_delay_ms(0.5*2);
    }
    //go off (inverted mode)
	for (j=0; j <= 255; j++) {
		OCR1B=j;
		_delay_ms(0.5*2);
    }  
  }
 	
 GTCCR = dummy_GTCCR; //restore, since I could not use another pin as output (no idea why)
 TCCR1 = dummy_TCCR1; //restore, since I could not use another pin as output (no idea why) 
 	
}


///////////////////////////////////////
//void send_data()
///////////////////////////////////////
/* Uncomment if you want to use. Commented to make the hex file as small as possible
void send_data(uint16_t data){
//sends data via SPI (own protocol)
//output MISO pin6 --> GPIO09 board pin21 on raspberry
//clock in MOSI input 5 --> GPIO10 board pin19 on raspberry

pinMode(PB0, INPUT); //pin5 --> this is actually input
pinMode(PB1, OUTPUT); //pin6
pinMode(PB4, OUTPUT); //pin3 used for transmission indication

digitalWrite(PB1, LOW);
  
uint8_t bit_count=0;

 for(bit_count=0;bit_count<16;bit_count++){
  //wait for clock to get high
  while(!digitalRead(PB0)){
    digitalWrite(PB4,HIGH); //pin3
  }
  //send the first bit LSB
  digitalWrite(PB1, (data >> bit_count) & 1);
  
  //wait for clock to get low
  while(digitalRead(PB0)){
    digitalWrite(PB4,LOW); //pin3
  }
 }

}

*/

void system_sleep() {
 
 //think about pins states for power consumption reduction
  
 ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
 power_all_disable(); //shuts down everything
  
  //backup status of registers
  uint8_t DDRB_backup=DDRB;
  uint8_t PORTB_backup=PORTB;
  uint8_t PINB_backup=PINB;
  
 // Ensure no floating
 DDRB = 0xFF; // PORTB is output, all pins
 PORTB = 0x00; // Make pins low
 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  
  sei(); //enable interrupts so that we can wake-up
  sleep_mode();                        // System actually sleeps here
  cli(); //disable interrupts so that we are not interrupted doing work
  sleep_disable();                     // System continues execution here when watchdog timed out 
  
  //restore registers
  DDRB=DDRB_backup;
  PORTB=PORTB_backup;
  PINB=PINB_backup;
  
   power_all_enable(); //start everything
   ADCSRA |= (1<<ADEN); //Enable ADC  

}

///////////////////////////////////////
//void show_measurement()
///////////////////////////////////////
/* Uncomment if you want to use. Commented to make the hex file as small as possible
void show_measurement() {
 float adc_step=1.1/1023;  // (0-5V over 1024 values)
 float voltage_fl;        // real battery voltage (0-5V) with decimals
    
   
 voltage_fl = read_10bit_ADC() * adc_step;

  if(voltage_fl <= 1){ //resistor divider to make sure 4v equals to 11.4 in total
    blink_led_x_times(1);
  }else if(voltage_fl > 1 && voltage_fl <= 2){ //resistor divider to make sure 4v equals to 11.4 in total
    blink_led_x_times(2);
  }else if(voltage_fl > 2 && voltage_fl <= 3){ //resistor divider to make sure 4v equals to 11.4 in total
    blink_led_x_times(3);
  }else if(voltage_fl > 3 && voltage_fl <= 4){ //resistor divider to make sure 4v equals to 11.4 in total
    blink_led_x_times(4);
  }else if(voltage_fl > 4){ //resistor divider to make sure 4v equals to 11.4 in total
    blink_led_x_times(5);
   }
}
*/


// Convert normal decimal numbers to binary coded decimal
uint8_t decToBcd(uint8_t val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
uint8_t bcdToDec(uint8_t val)
{
  return( (val/16*10) + (val%16) );
}

//RTC set function
void setDS3231time(uint8_t power_pin, uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
dayOfMonth, uint8_t month, uint8_t year)
{

  i2c_begin(); //init registers

  pinMode(power_pin, OUTPUT);
  //power on RTC
  digitalWrite(power_pin,HIGH);
  _delay_ms(300); // wait some time according to the datasheet
   
  // sets time and date data to DS3231
  i2c_beginTransmission(DS3231_I2C_ADDRESS);
  i2c_send(0); // set next input to start at the seconds register
  i2c_send(decToBcd(second)); // set seconds
  i2c_send(decToBcd(minute)); // set minutes
  i2c_send(decToBcd(hour)); // set hours 0-23 If you want 12 hour am/pm you need to set
                                   // bit 6 (also need to change readDateDs1307)
  i2c_send(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  i2c_send(decToBcd(dayOfMonth)); // set date (1 to 31)
  i2c_send(decToBcd(month)); // set month
  i2c_send(decToBcd(year)); // set year (0 to 99)
  i2c_endTransmission();
    //power off RTC
  digitalWrite(power_pin,LOW);
  pinMode(power_pin, INPUT);  //otherwise during led blinking we get power. Do not ask me why.
}


//RTC read function
void readDS3231time(uint8_t power_pin, uint8_t *second,
uint8_t *minute,
uint8_t *hour,
uint8_t *dayOfWeek,
uint8_t *dayOfMonth,
uint8_t *month,
uint8_t *year)
{

  i2c_begin(); //init registers
  
  pinMode(power_pin, OUTPUT);
  //power on RTC
  digitalWrite(power_pin,HIGH);
  _delay_ms(300); // wait some time according to the datasheet
	
  i2c_beginTransmission(DS3231_I2C_ADDRESS);
  i2c_send(0); // set DS3231 register pointer to 00h
  i2c_endTransmission();
   // request seven bytes of data from DS3231 starting from register 00h
  i2c_requestFrom(DS3231_I2C_ADDRESS, 7);
 
   // A few of these need masks because certain bits are control bits
  *second = bcdToDec(i2c_receive() & 0x7f);
  *minute = bcdToDec(i2c_receive());
  *hour = bcdToDec(i2c_receive() & 0x3f); //// Need to change this if 12 hour am/pm required
  *dayOfWeek = bcdToDec(i2c_receive());
  *dayOfMonth = bcdToDec(i2c_receive());
  *month = bcdToDec(i2c_receive());
  *year = bcdToDec(i2c_receive());
  
//power off RTC
  digitalWrite(power_pin,LOW);
    pinMode(power_pin, INPUT); //otherwise during led blinking we get power. Do not ask me why.


}

//function to find first available free cell (0xFF)
uint16_t return_eeprom_adr(){
uint16_t eeprom_adr=0;
for(eeprom_adr=0;eeprom_adr<MAX_SIZE;eeprom_adr++){
 //check for three 0xFF in a row 
 if(eeprom_read_byte((uint8_t *)(uint16_t) eeprom_adr) == 0xFF && eeprom_read_byte((uint8_t *)(uint16_t) ((eeprom_adr+1)%MAX_SIZE)) == 0xFF && eeprom_read_byte((uint8_t *)(uint16_t) ((eeprom_adr+2)%MAX_SIZE)) == 0xFF ){
 	return eeprom_adr;
  }
 }
 return 0; //if nothing found
}



/*void log_vcc() {
 //init and enable ADC on Bandgap (1.1)
 //VCC ref voltage is set
 initADC_vcc();
 
 //raw_adc = 1023 * (1.1 / VCC)
 //VCC = 1023*1.1 / raw_adc --> be carefull with division by zero

 uint16_t raw_adc=0;
 uint8_t raw_adc_byte=0;
       
 raw_adc=read_10bit_ADC();

 raw_adc_byte = raw_adc>>2; //take 8 bit instead of 10 to save memory -->  VCC = 255*1.1 / raw_adc --> be carefull with division by zero
 eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, raw_adc_byte); 
}*/


///////////////////////////////////////
//void show_temp()
///////////////////////////////////////
/*void show_temp(){
uint8_t temp=read_temp();

if(temp<10){
	blink_led_x_times(1);
 }else if(temp > 10 && temp <= 15){
    blink_led_x_times(2);
 }else if(temp > 15 && temp <= 20){
    blink_led_x_times(3);
 }else if(temp > 20){
    blink_led_x_times(4);
 }

}*/

/*void log_temp(){
 initADC_temp();
 //BE AWARE the read_temp() is a SIGNED int8_t   
 //download the eeprom content by using "make eeprom"   
 eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, read_temp()); 
}*/


///////////////////////////////////////
//void show_low_voltage()
///////////////////////////////////////

/*void show_low_voltage() {
 //init and enable ADC on PB3 (Pin 2)
 //1.1V ref voltage is set
 initADC_10bit();

 float adc_step=1.1/1023;  //for 1.1V ref voltage
 float voltage_fl;        // real battery voltage (0-1.1V) with decimals
 uint16_t raw_adc=0;
 uint8_t raw_adc_byte=0;
       
 raw_adc=read_10bit_ADC();
 voltage_fl = raw_adc * adc_step;
 raw_adc_byte = raw_adc>>2; //take 8 bit instead of 10 to save memory
 eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, raw_adc_byte); 

 if(voltage_fl <= 0.748){ //resistor divider (1,013M + 0,47M ---- 0,0995M for ADC) equals to 11,9V in total
	//before stopping make sure that the triple (ADC, VCC, Temp) is not broken.
    eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
    log_vcc();
    eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
    log_temp();
    eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
    
    //make three coming cells "empty", but do not change the eeprom_adr, where we will continue next time
    eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0xFF); //empty next cell due to ring buffer 
    eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+1)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
    eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+2)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
    
   for(;;){
    //start blinking on pin3, PB4
	blink_led_x_times(3);
    system_sleep(); //Go to sleep! Wake up after watchdog time
    }
   }  
}
*/

///////////////////////////////////////
//void log_voltage()
///////////////////////////////////////
/*void log_voltage() {

 //init and enable ADC on PB3 (Pin 2)
 //1.1V ref voltage is set
 initADC_10bit();

 uint16_t raw_adc=0;
 uint8_t raw_adc_byte=0;
       
 raw_adc=read_10bit_ADC();
 raw_adc_byte = raw_adc>>2; //take 8 bit instead of 10 to save memory
 eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, raw_adc_byte); 


}*/

///////////////////////////////////////
//void log_date_time()
///////////////////////////////////////
/*void log_date_time() {

uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
readDS3231time(PB1, &second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);

eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, dayOfMonth); //write day of month part of the triple 
eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, month); //write month part of the triple 
eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
//eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, year); //write year part of the triple 
//eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, hour); //write hour in the first part of the triple 
eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, minute); //write minute in the first part of the triple 
eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
//eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, second); //write second in the first part of the triple 
//eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
 
}*/

void loop() {

/*    log_date_time();
 	//start measuring without waiting for one hour
    //show_low_voltage();
    log_voltage();
    eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
    log_vcc();
    eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
    log_temp();
    eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
    
    //make three coming cells "empty", but do not change the eeprom_adr, where we will continue next time
    eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0xFF); //empty next cell due to ring buffer 
    eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+1)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
    eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+2)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
*/

uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month, year;
uint16_t dummy_voltage = 0;
int8_t dummy_temp = 0;
  // retrieve data from DS3231
readDS3231time(PB1, &second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
recording_day = dayOfMonth;
recording_month = month;

 for(;;){

  system_sleep(); //Go to sleep! Wake up after watchdog time
  if(watchdog_counter > 75*6) {//75*6 --> aprx 1 hour //75*6*6 = are 6,5 hours passed? (1:04:58 * 6 --> as per calculations)

    watchdog_counter = 0;

	initADC_10bit();
	dummy_voltage = read_10bit_ADC();
 	min_v= MIN( (dummy_voltage>>2), min_v);
 	initADC_vcc();
 	dummy_voltage = read_10bit_ADC();
 	min_vcc= MIN( (dummy_voltage>>2), min_vcc);
 	initADC_temp();
 	dummy_temp = read_temp();
 	max_temp = MAX(dummy_temp, max_temp);
 	min_temp = MIN(dummy_temp, min_temp);
 	readDS3231time(PB1, &second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
 	reading++;
 	reading = MIN(reading, UINT8_MAX-1); //prevent overflow
 
 	//if the previous day is over check whether enough measurements are done
	if(dayOfMonth != recording_day){
		if(reading > MIN_READING){
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, recording_day); //write day of month part of the triple 
		eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, recording_month); //write month part of the triple 
		eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, min_v); //write min voltage during that day 
		eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, min_vcc); //write min vcc during that day 
		eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, max_temp); //write max temperature during that day 
		eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, min_temp); //write min temperature during that day 
		eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
		eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0xFF); //empty next cell due to ring buffer 
    	eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+1)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
    	eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+2)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
		}
		//Independent of enough measurements are available after day change reset everything	
		readDS3231time(PB1, &second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
		recording_day = dayOfMonth;
		recording_month = month;
		min_temp = INT8_MAX; //0x7f;
		max_temp = INT8_MIN; //(-INT8_MAX - 1)
		min_vcc = UINT8_MAX;
		min_v = UINT8_MAX;
		reading = 0;
		initADC_10bit();
		dummy_voltage = read_10bit_ADC();
 		min_v= MIN( (dummy_voltage>>2), min_v);
 		initADC_vcc();
 		dummy_voltage = read_10bit_ADC();
 		min_vcc= MIN( (dummy_voltage>>2), min_vcc);
 		initADC_temp();
 		dummy_temp = read_temp();
 		max_temp = MAX(dummy_temp, max_temp);
 		min_temp = MIN(dummy_temp, min_temp);
 		reading++;
	}
 
     //show_measurement(); //Do the periodic job
     //log_date_time();
     //show_low_voltage();
     //log_voltage();
     //eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
     //log_vcc();
     //eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
     //log_temp();
     //eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
   
   //make three coming cells "empty", but do not change the eeprom_adr, where we will continue next time
    //eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0xFF); //empty next cell due to ring buffer 
    //eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+1)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
    //eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+2)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
  }
  
/*  if(int_flag != 0){
   int_flag=0;
   //show that interrupt is catched
    blink_led_x_times(1);
  }*/
  
 }

}


/*void candle_light_loop(){

	init_rng(1,2,3);
	
   //and set PORTB4 as output for led blinking
 	pinMode(PB4, OUTPUT); //pin3
 	
 	uint8_t dummy_GTCCR = GTCCR; //backup
 	uint8_t dummy_TCCR1 = TCCR1; //backup
  //PWM configuration
  //be careful before changing timer pre-scalars since other functions like _delay_ms relay on them
  // Configure counter/timer0 for fast PWM on PB0 and PB1
  //TCCR0A = 3<<COM0A0 | 3<<COM0B0 | 3<<WGM00;
  //TCCR0B = 0<<WGM02 | 3<<CS00; // Optional; already set
  // Configure counter/timer1 for fast PWM on PB4
  OCR1B=255; //make sure that output does not go on high during init
  GTCCR = 1<<PWM1B | 3<<COM1B0;
  TCCR1 = 3<<COM1A0 | 7<<CS10;
 	
 	//PWM configuration	
	//Output PB0: OCR0A or OCR1A.
	//Output PB1: OCR0B or OCR1A.
	//Output PB2: none.
	//Output PB3: OCR1B.
	//Output PB4: OCR1B.
 	
  uint8_t j=255;    
  uint8_t rnd;
  for(;;){
	rnd = (uint8_t) getRandom();
  	//start blinking smoothly
  	if(j > rnd){
		for (; j > rnd; j--) {
			OCR1B=j;
			_delay_ms(0.5*2);
     	}
     }
	else if(j < rnd){
		for (; j < rnd; j++) {
			OCR1B=j;
			_delay_ms(0.5*2);
     	} 
    }
	j=rnd; 
  }
 	
 GTCCR = dummy_GTCCR; //restore, since I could not use another pin as output (no idea why)
 TCCR1 = dummy_TCCR1; //restore, since I could not use another pin as output (no idea why) 
 	
}
*/


/*
			PCINT5, Reset  1 *      8  VCC 5 Volts
PCINT3, Analog In 3, PB3   2        7  PB2, Analog In 1, I2C SCL, SPI SCK, PCINT2
PCINT4, Analog In 2, PB4   3        6  PB1, PWM, SPI MISO, PCINT1
					Ground 4        5  PB0, PWM, I2C SDA, SPI MOSI, PCINT0
*/

int main() {

//Bit0: Power on reset
//Bit1: External reset
//Bit2: Brown-out reset
//Bit3: Watchdog reset
//Bit4-7: Reserved
//uint8_t MCU_Status = MCUSR; //MCU status (external reset, power on, brown out etc.)
//MCUSR=0x00; //reset the status

//general interrupt mask register
//set the pin interrupt
/////////////////////////////////////////GIMSK |= (1<<PCIE);
//pin change mask register
//set interrupt for PCINT2 --> PB2 --> Pin7
/////////////////////////////////////////PCMSK |= (1<< PCINT2);

setup_watchdog(9); //Setup watchdog to go off after 8seconds (max possible)


//indicate that attiny85 has started working
blink_led_x_times(7);
 //init eeprom_adr by finding the first available slot
 eeprom_adr = return_eeprom_adr();



 // set date and time on DS3231
 //sec,min,hour,day of week (1=Sunday),day,month,year
//setDS3231time(PB1, 0,37,15,1,21,07,19); //pin6

		

//DEBUG
//eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, MCU_Status); //write MCU status in the first part of the triple 
//eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
//eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0x00); //first part of the magic pattern 
//eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
//eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0xAA); //second part of the magic pattern 
//eeprom_adr=((eeprom_adr+1)%MAX_SIZE); //if adr = MAX_SIZE than set to zero
//make three coming cells "empty", but do not change the eeprom_adr, where we will continue next time
//eeprom_update_byte((uint8_t *)(uint16_t) eeprom_adr, 0xFF); //empty next cell due to ring buffer 
//eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+1)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
//eeprom_update_byte((uint8_t *)(uint16_t) ((eeprom_adr+2)%MAX_SIZE), 0xFF); //empty next cell due to ring buffer 
//END OF DEBUG


 loop();

//to send data to raspberry (start get_data.py for voltage and get_data2.py for int on raspberry)
//pinMode(PB4, OUTPUT); 
//digitalWrite(PB4,HIGH); //pin3

//send_data(0xFFFF);
//send_data(0x0000);
//for(;;){
//send_data(read_10bit_ADC());
//}
//blink_led();


return(0);
}
