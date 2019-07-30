
//gcc read_voltage.c -lm -lwiringPi -o read_voltage


#include <wiringPiSPI.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define HIGH_PRECISION //DECIMATION ENABLED
//#undef HIGH_PRECISION //DECIMATION DISABLED

#ifdef HIGH_PRECISION
 #define N 5 //additional bits to achieve by decimation
#else
 #define N 0 //additional bits to achieve by decimation
#endif

const unsigned int R1=22010; //voltage divider --> measured part
const unsigned int R2=99400; //voltage divider

// channel is the wiringPi name for the chip select (or chip enable) pin.
// Set this to 0 or 1, depending on how it's connected.
const int CHANNEL = 0;

void ADC_decimation(float *result_voltage, const float v_ref){
//clock_t start, end;
//double cpu_time_used;
//start = clock();

//total amount of run time for one sample step is aprx. 31 us.
//Use decimation:
//1)collect 4^n samples to increase the resolution by n bits.
//2) Take sum of all samples.
//3) Now right shift the sum by n bits (or divide by 2^n).
//4) The result is a ADC binary reading of 8 + n bits.
//5) recalculate the vref for 8 + n bits.
//6) Multiply the result with the new vref.

  unsigned char result[2]={0,0};
  unsigned long int sum[2]={0,0};
  unsigned int n=N; //by number of bits to increase the sampling through decmation
  unsigned int count=pow(4,n);

for(;count>0;count--)
 {
  //if wanted two channels can be read
  result[0]=0;
  result[1]=0;

  // Read  TCL549 conversion
  wiringPiSPIDataRW(CHANNEL, &result[0], 1);

  sum[0]=sum[0]+result[0];
  sum[1]=sum[1]+result[1];
}

sum[0]=sum[0]/pow(2,n);
sum[1]=sum[1]/pow(2,n);

result_voltage[0] = (float) (sum[0]*v_ref/R1*(R1+R2)*1000)/pow(2,n+8); //convert the bit number to voltage
result_voltage[1] = (float) (sum[1]*v_ref/R1*(R1+R2)*1000)/pow(2,n+8); //convert the bit number to voltage

//end = clock();
//cpu_time_used = ((double) (end - start))/ CLOCKS_PER_SEC;
//printf("**%f seconds**\n",cpu_time_used);
//fflush(stdout);
return;

}

int main(int argc, char *argv[]){

float result[2]={0,0};
float last_result[2]={0,0};
float moving_average[2][5]; //array to hold 5th order of moving average of both results
time_t current;
struct tm *tblock;
char spinner[]="\\-/|";
unsigned int counter=0;
unsigned int i=0;
const float v_ref= 3.3;  //reference voltage
const float min_resol= ((v_ref/R1*(R1+R2)*1000)/pow(2,8+N));


// Configure the interface.
// CHANNEL insicates chip select,
// 500000 indicates bus speed.
//could go upto 32000000 but not with this chip
wiringPiSPISetup(CHANNEL, 500000);

//start logging

ADC_decimation(last_result, v_ref);
for(i=0;i<5;i++){ //init the moving average array
moving_average[0][i]=last_result[0]/5;
moving_average[1][i]=last_result[1]/5;
}
current = time(NULL);
tblock = localtime(&current);

if(argc == 2){
 if(!strcmp(argv[1],"--no-recursion")){
  printf("%02d.%02d.%04d %02d:%02d:%02d;",tblock->tm_mday,tblock->tm_mon+1,tblock->tm_year+1900,tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
  printf(" (Resolution: %.0f mV); Port1(mV): ;%.0f; Port2(mV): ;%.0f\n",min_resol,last_result[0], last_result[1]);
  fflush(stdout);
  //a dirty shutdown mechanism without physical buttons and ssh
  if(last_result[0]==0 && last_result[1]==0){
   sleep(5); //sleep for 5 seconds
   //confirm that the value is steady and zero
   ADC_decimation(last_result, v_ref);
   if(last_result[0]==0 && last_result[1]==0){
    printf("Shutting down\n");
    system("sudo poweroff");
   }
  }
  return 0;
 }
}
for(;;counter++){ //counter is used for two different tasks! but no matter.
ADC_decimation(result, v_ref);
moving_average[0][counter%5]=result[0]/5;
moving_average[1][counter%5]=result[1]/5;
result[0]=0;
result[1]=0;

for(i=0;i<5;i++){ //compute the moving average
result[0]=result[0]+moving_average[0][i];
result[1]=result[1]+moving_average[1][i];
}

   if(abs(last_result[0]-result[0])> min_resol || abs(last_result[1]-result[1])> min_resol) {
   current = time(NULL);
   tblock = localtime(&current);
   printf("\r%02d.%02d.%04d %02d:%02d:%02d",tblock->tm_mday,tblock->tm_mon+1,tblock->tm_year+1900,tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
   printf(" (Resolution: %.0f mV) Port1: %.3f V Port2: %.3f V\t%c  ",min_resol,result[0]/1000, result[1]/1000, spinner[counter%4]);
   fflush(stdout);
   last_result[0]=result[0];
   last_result[1]=result[1];
 }

 usleep(60000000); //sleep for 60 seconds

}

return 0;
}
