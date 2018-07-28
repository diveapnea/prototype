
#include <stdio.h>
#include <stdint.h>


#define GENOME_SIZE 5
#define MEM_SIZE 1000
#define MAX_ORGANISM 200

//genome instruction set
#define NOP 0
#define MOVFWD 1
#define MOVAFT 2
#define CPFWD 3
#define CPAFT 4
#define MUTATE 5
#define XGROW 6 //superior DNA. Just to check how it overcomes other DNAs
#define FAKE 7 //fake something (i.e. signature to come off big signature death)
#define LIVE_LOW 8 //can only live in the lower part of the memory
#define MAX_CMD 9 //biggest command number + 1 for modulation


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




class Organism {
   public:
   
     uint8_t genome[GENOME_SIZE];     
	 uint32_t signature; //to identify the organism
     uint32_t score; 
     uint8_t lifetime;   
     uint8_t null_organism; 

     void Init () {
     	uint16_t i=0;
     	for(i=0;i<GENOME_SIZE;i++){
  			genome[i] = getRandom()%MAX_CMD;
  		}
		SetSignature();
		score = 0;
		lifetime = 0;
     }
     
     void InitNull () {
     	uint16_t i=0;
     	for(i=0;i<GENOME_SIZE;i++){
  			genome[i] = NOP;
  		}
		SetSignature();
		score = 0;
		lifetime = 0;
     }
     
     // Fetch AND EXECUTE next 'command' from prn (applying current range)
     // Importantly, a cmd can match *more than one* instruction!
     void Command (uint16_t curr_eeprom, Organism *eeprom[]) {
     	uint16_t i=0;
     	for(i=0;i<GENOME_SIZE;i++){
  			switch(genome[i]%MAX_CMD){
				case NOP: break;
				case MOVFWD: eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+1)%MEM_SIZE]); eeprom[curr_eeprom]->InitNull(); break;
				case MOVAFT: eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+MEM_SIZE-1)%MEM_SIZE]); eeprom[curr_eeprom]->InitNull(); break;
				case CPFWD: eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+1)%MEM_SIZE]); break;
				case CPAFT: eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+MEM_SIZE-1)%MEM_SIZE]); break;
				case MUTATE: Mutate(); break;
				case XGROW: eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+1)%MEM_SIZE]); eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+2)%MEM_SIZE]); eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+MEM_SIZE-1)%MEM_SIZE]); eeprom[curr_eeprom]->CrossOver(eeprom[(curr_eeprom+MEM_SIZE-2)%MEM_SIZE]); break;
				case FAKE: signature=1; break;	
				case LIVE_LOW: if(curr_eeprom > (MEM_SIZE)/2) eeprom[curr_eeprom]->InitNull(); break;
				default: break;
			}
  		}
     
     }    
     
     void Mutate () {
     	genome[getRandom()%GENOME_SIZE]= getRandom()%MAX_CMD;
   		SetSignature();  	
     }                                             
     
     void Clone (Organism *org) {       
        uint16_t i=0;
     	for(i=0;i<GENOME_SIZE;i++){
			org->genome[i] = genome[i]%MAX_CMD;
  		}
		org->SetSignature();
     }
     
     void CrossOver (Organism *org) {  
     	if(org->signature == 0) Clone(org);
     	else{     
        	uint16_t i=0;
     		for(i=0;i<GENOME_SIZE;i++){
     	   		if(i%2 == 1) genome[i] = org->genome[i]%MAX_CMD;
     	   	else org->genome[i] = genome[i]%MAX_CMD;
  			}
  		org->SetSignature();
  		SetSignature();
  		}
     }
     
     void PrintGenome(){
     	uint16_t i=0;
     	for(i=0;i<GENOME_SIZE;i++){
			printf("%u-", genome[i]%MAX_CMD);	
		}
     }
     
     void SetSignature(){
		uint16_t i=0;
		signature = 0;
     	for(i=0;i<GENOME_SIZE;i++){
  			signature += (genome[i]%MAX_CMD)<<i;
  		}
	
     }
                                        
     
};

Organism *eeprom[MEM_SIZE];


void loop() {

uint8_t end=0;
uint16_t i=0;
uint16_t j=0;
uint64_t n=0;

 for(n=0;n<0xFFFFFFFFFF;n++){

	i=(i+getRandom())%MEM_SIZE;
	eeprom[i]->Command(i, eeprom);
	
	//rule for death
	if(eeprom[i]->signature > 20) eeprom[i]->InitNull();



	//exit if no null_organism left
	/*end = 0;
	for(j=0;j<MEM_SIZE;j++){
		if(eeprom[j]->signature == 0) end=1;
	}
	
	if(end == 0) break;*/
	
	//exit if only null_organism left
	end = 0;
	for(j=0;j<MEM_SIZE;j++){
		if(eeprom[j]->signature != 0) end=1;
	}

	if(end == 0) break;
	
	//exit if only one DNA sequence type is left
	end = 0;
	for(j=0;j<MEM_SIZE;j++){
		if(eeprom[j]->signature != eeprom[(j+1)%MEM_SIZE]->signature) end=1;
	}

	if(end == 0) break;
	
	if((n%0xFFFFFF)==0){
		for(i=0;i<MEM_SIZE;i++){
			printf("(%u) ",eeprom[i]->signature);
		}
		printf("\nAnlik durum\n");
	}

	
 }
  

}



int main() {


//INIT
init_rng(1,2,3);


uint16_t i=0;

//init with null_organism first
for(i=0;i<MEM_SIZE;i++){
	eeprom[i] = new Organism;
	eeprom[i]->InitNull();
}

//in this manner the distribution of organisms is better
for(i=0;i<MAX_ORGANISM;i++){
	eeprom[getRandom()%MEM_SIZE]->Init();
}

for(i=0;i<MEM_SIZE;i++){
	if(eeprom[i]->signature !=0){
		if(i!=0){
			if(eeprom[i]->signature != eeprom[i-1]->signature){
				printf("%u: ",eeprom[i]->signature);	
 				eeprom[i]->PrintGenome();
 				printf("\n");
 			}
 		}else{
 			printf("%u: ",eeprom[i]->signature);
 			eeprom[i]->PrintGenome();
 			printf("\n");
 		}
 	}

}

loop();

for(i=0;i<MEM_SIZE;i++){
	printf("(%u) ",eeprom[i]->signature);
}

printf("\n");
for(i=0;i<MEM_SIZE;i++){
	if(eeprom[i]->signature !=0){
		if(i!=0){
			if(eeprom[i]->signature != eeprom[i-1]->signature){
				printf("%u: ",eeprom[i]->signature);	
 				eeprom[i]->PrintGenome();
 				printf("\n");
 			}
 		}else{
 			printf("%u: ",eeprom[i]->signature);
 			eeprom[i]->PrintGenome();
 			printf("\n");
 		}
 	}

}


printf("\nBitti.\n");

   		
return(0);


}
