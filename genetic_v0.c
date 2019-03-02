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



// An example of a simple pseudo-random number generator is the
// Multiply-with-carry method invented by George Marsaglia.
// two initializers (not null)
unsigned long m_w = 1;
unsigned long m_z = 2;


///////////////////////////////////////
//unsigned long getRandom()
///////////////////////////////////////
unsigned long getRandom()
{
    m_z = 36969L * (m_z & 65535L) + (m_z >> 16);
    m_w = 18000L * (m_w & 65535L) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

uint8_t digitalRead(uint8_t pin){
 return (PINB & (1<<pin));
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
  	for(i=0;i<MEM_SIZE;i++){

    score += gene[i]; 
  
   }
 
return score;

}

void init_gene(){


	eeprom_read_block(&gene, 0, MEM_SIZE); 
  	
 	previous_score=get_score();
 
}

void mutate_gene(){

gene[getRandom()%MEM_SIZE]= (uint8_t) (getRandom()%0xFF);

}

void regen_gene(){

uint16_t i=0;
uint16_t j=0;

for(j=0;j<200;j++){
  	for(i=0;i<MEM_SIZE;i++){
  		gene[i] = (uint8_t) (getRandom()%0xFF);
  	}
  	if(get_score() < previous_score){
		eeprom_update_block(&gene, 0, MEM_SIZE); 
		previous_score = get_score();
   		break;
   	}
}

gene_lifetime = 0;

}


void loop() {

 for(;;){


	mutate_gene();

	//target is get_score()==0
	if(get_score() < previous_score){
	 
		previous_score = get_score();

   		eeprom_update_block(&gene, 0, MEM_SIZE); 
	} else init_gene();
	
	if(previous_score < 10) break;
	
	gene_lifetime++;
	if(gene_lifetime > 200) regen_gene();
	
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
init_gene();  

loop();

eeprom_update_byte(511, 0x00); //for me to see
for(;;) blink_led_x_times(1);
   		
return(0);
}
