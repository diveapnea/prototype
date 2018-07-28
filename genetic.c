#include <avr/io.h>
// F_CPU frequency to be defined at command line
#include <util/delay.h>
//#include <avr/sleep.h>
//#include <avr/wdt.h> //Needed to enable/disable watch dog timer
//#include <avr/interrupt.h>      // library for interrupts handling
//#include <util/delay.h>
#include <avr/eeprom.h>
//#include <avr/power.h>  //power saving functions
//#include "TinyWireM.h"
//#include <stdio.h>
//#include <stdint.h>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define MEM_SIZE 256

uint8_t gene[MEM_SIZE];
uint8_t gene_lifetime=0;
uint32_t previous_score=0;


//This generator is free to use, but is not suitable for cryptography due to its short period(by //cryptographic standards) and simple construction. No attempt was made to make this generator
// suitable for cryptographic use.
//
//Due to the use of a constant counter, the generator should be resistant to latching up.
//A significant performance gain is had in that the x variable is nly ever incremented.
//
//Only 4 bytes of ram are needed for the internal state, and generating a byte requires 3 XORs , //2 ADDs, one bit shift right , and one increment. Difficult or slow operations like multiply, etc
//were avoided for maximum speed on ultra low power devices.

uint8_t a_rnd=0,b_rnd=0,c_rnd=0,x_rnd=0;

void init_rng(uint8_t s1, uint8_t s2, uint8_t s3) //Can also be used to seed the rng with more entropy during use.
{
//XOR new entropy into key state
a_rnd ^=s1;
b_rnd ^=s2;
c_rnd ^=s3;

x_rnd++;
a_rnd = (a_rnd^c_rnd^x_rnd);
b_rnd = (b_rnd+a_rnd);
c_rnd = c_rnd+((b_rnd>>1)^a_rnd);
}

uint8_t getRandom()
{
x_rnd++;               //x is incremented every round and is not affected by any other variable
a_rnd = (a_rnd^c_rnd^x_rnd);       //note the mix of addition and XOR
b_rnd = (b_rnd+a_rnd);         //And the use of very few instructions
c_rnd = c_rnd+((b_rnd>>1)^a_rnd);  //the right shift is to ensure that high-order bits from b can affect  
return(c_rnd);         //low order bits of other variables
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



uint32_t get_score(){

uint32_t score=0;
uint16_t i=0;
uint8_t previous;
uint8_t max_available=0;
uint8_t min_available=0;

  	for(i=0;i<MEM_SIZE;i++){
		//try to learn smooth blinking led
  		previous=gene[(i+1%MEM_SIZE)];
		if(previous > gene[i]) score += (previous-gene[i]-1);
		else score += (gene[i]-previous-1);
		if(gene[i] == previous) score += 0xFF;
  		if(gene[i] == 0xFF) max_available +=1;
  		if(gene[i] == 0x00) min_available +=1; 
	}
 	
 	if(max_available != 0) score += 0xFF;
 	if(min_available != 0) score += 0xFF;
 	
return score;

}


void init_gene(){


	eeprom_read_block(&gene, 0, MEM_SIZE); 
  	
 	previous_score=get_score();
 
}

void mutate_gene(){

gene[getRandom()%MEM_SIZE]= getRandom();

}

void regen_gene(){

uint16_t i=0;
uint16_t j=0;

for(j=0;j<200;j++){
  	for(i=0;i<MEM_SIZE;i++){
  		gene[i] = getRandom();
  	}
  	if(get_score() < previous_score){
		eeprom_update_block(&gene, 0, MEM_SIZE); 
		previous_score = get_score();
   		break;
   	}
}

eeprom_read_block(&gene, 0, MEM_SIZE);
previous_score = get_score();
gene_lifetime = 0;

}


void loop() {

   //and set PORTB4 as output for led blinking
 	pinMode(PB4, OUTPUT); //pin3
 	
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
 uint32_t count;	
 for(count=0;count<0xFFFFFFFF;count++){


	mutate_gene();

	//target is get_score()==0
	if(get_score() < previous_score){
	 
		previous_score = get_score();

   		eeprom_update_block(&gene, 0, MEM_SIZE); 
	} else init_gene();
	
	if(previous_score < 1) break;
	
	gene_lifetime++;
	if(gene_lifetime > 200) regen_gene();
	
	if(count%0xFF==0){
	//try the genome
		uint16_t i=0;
  		for(i=0;i<MEM_SIZE;i++){
			OCR1B=gene[i];
			_delay_ms(0.5*2);
		}  		
	}
 }
  

}




/*
			PCINT5, Reset  1 *      8  VCC 5 Volts
PCINT3, Analog In 3, PB3   2        7  PB2, Analog In 1, I2C SCL, SPI SCK, PCINT2
PCINT4, Analog In 2, PB4   3        6  PB1, PWM, SPI MISO, PCINT1
					Ground 4        5  PB0, PWM, I2C SDA, SPI MOSI, PCINT0
*/

int main() {

//blink_led_x_times(1);


//INIT
init_rng(1,2,3);
init_gene();  

loop();

eeprom_update_byte((uint8_t *)511, 0x00); //for me to see
for(;;) blink_led_x_times(1);
   		
return(0);
}
