//#include <avr/io.h>
// F_CPU frequency to be defined at command line
//#include <util/delay.h>
//#include <avr/sleep.h>
//#include <avr/wdt.h> //Needed to enable/disable watch dog timer
//#include <avr/interrupt.h>      // library for interrupts handling
//#include <util/delay.h>
//#include <avr/eeprom.h>
//#include <avr/power.h>  //power saving functions
//#include "TinyWireM.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define MEM_SIZE 256

uint8_t gene[MEM_SIZE];
uint8_t gene_lifetime=0;
uint32_t previous_score=0;
uint8_t eeprom[MEM_SIZE];
uint64_t all=0;



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


uint32_t get_score(){

uint32_t score=0;
uint16_t i=0;
  	for(i=0;i<MEM_SIZE;i++){

    score += gene[i]; 
  
   }
 
return score;

}

void init_gene(){


	//eeprom_read_block(&gene, 0, MEM_SIZE); 
  	strncpy(gene,eeprom, MEM_SIZE);
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
		//eeprom_update_block(&gene, 0, MEM_SIZE); 
		strncpy(eeprom,gene, MEM_SIZE);
		previous_score = get_score();
		all++;
   		break;
   	}
}

gene_lifetime = 0;

}


void loop() {

 for(;;){

	all++;
	mutate_gene();

	//target is get_score()==0
	if(get_score() < previous_score){
	 
		previous_score = get_score();

   		//eeprom_update_block(&gene, 0, MEM_SIZE); 
   		strncpy(eeprom,gene, MEM_SIZE);
	} else init_gene();
	
	if(previous_score < 1) break;
	
	gene_lifetime++;
	if(gene_lifetime > 200) regen_gene();
	
 }
  

}




int main() {


uint16_t i=0;
for(i=0;i<MEM_SIZE;i++){
  	eeprom[i] = 0xFF;
 }

for(i=0;i<MEM_SIZE;i++){
  	printf("%X ",eeprom[i]);
 }


//INIT
init_gene();  

loop();

printf("Bitti. %lu %lu\n",previous_score, all);

for(i=0;i<MEM_SIZE;i++){
  	printf("%X ",eeprom[i]);
 }

  printf("\n");

   		
return(0);
}
