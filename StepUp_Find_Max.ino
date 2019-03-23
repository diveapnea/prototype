//Atmega 328 tested
//5W 178ohm RJ
/* Test results (max_freq_divider, pwn_for_Vmax, duty cyclw, Vmax): 253 237 92.94% 19.13 (w/o load)
 *  Testing freq divider: 99 252 233 13.58 (/w 178ohm)
 *  Testing freq divider: 236 240 221 12.89 (/w 178ohm) - 2.4V


https://withinspecifications.30ohm.com/2014/02/20/Fast-PWM-on-AtMega328/

 * Pin 5 and 6 can go up tp 62,5kHz PWM
 * But the delay function becomes delay(62500) for 1 second
 * 0x01 ->62500hz
 * 0x02 -->7800hz
 * 0x03 -->980Hz (default for pin 5/6)
 * 0x04 -->250hz
 * 0x05 -->61hz
 * 
 * For pin 3,9,10,11 same thing with 31,3kHz max frequency
 * For pin3,9,10,11 counters would be TCCR1B and TCCR2B
 * 
 * Ideal combination: the biggest trafo (with most right pin) or the smallest coil
 * + 100uF 63V capacitor
 * +schottky 1N 5819
 * +IRFP4310ZPbF --> produces a lot of heat with pwm freq higher than 62,5kHz
 * --> gives 4,2 times voltage gain
 */

const int D5 = 5;
uint8_t pwm = 0;
uint8_t pwm_max = 0;
float Vmax = 0.0;
uint8_t pwm_for_Vmax = 0;
int max_freq_divider = 0;
const int decimation = 64; //4^x --> x bits of additional precision

void find_max(int freq_divider) {
  //start seeking for the highest voltage
  float A6_val=0; //output voltage 
  float A7_val=0; //input current 
  float A2_val=0; //input voltage

  OCR0A = freq_divider; //freq. divider // 1--> 8MHz,1bit pwm, 3-->4Mhz,2bit pwm, 7-->2Mhz,3bit pwm, 15-->1Mhz,4bit pwm etc.
  OCR0B = 0; //duty cycle (0 means 50% for 1 bit)
  pwm_max= OCR0A;
  
  for(pwm=0; pwm<pwm_max; pwm++) {
    analogWrite(D5, pwm);
    A6_val = analogRead(A6);
    //I/O over voltage protection
    if(A6_val > 1020){ //approaching saturation
      analogWrite(D5, 0);
      String result = String("Saturation (too high voltage) reached within: ") + freq_divider + String(" ") + pwm;
      Serial.println(result);
      while(true){}
    }
    
    //delay((float(freq_divider)*255-245)/500); // delay x ms --> 1 s becomes 62500 due to change of the clock multiplier
    A2_val = 0;
    A6_val = 0;
    A7_val = 0;
    for(int i=0;i<decimation;i++){
      // read the input on analog pin 6:
      A2_val = A2_val + analogRead(A2);
      A6_val = A6_val + analogRead(A6);
      A7_val = A7_val + analogRead(A7);
      }
    A2_val = A2_val/decimation;  
    A6_val = A6_val/decimation;  
    A7_val = A7_val/decimation;  

    //voltage divider used to sense 5V at 30V real//0.9961 is the calibration value
    float voltage_input = A2_val * 5.00 / 1023.0; 
    float voltage_output = A6_val * (30.0*0.9961/ 1023.0);
    float current_input = 0.05865*A7_val-30;
    float current_output = voltage_output/178.0; //load 178ohm
    
    if(voltage_output > Vmax) {
      Vmax = voltage_output;
      pwm_for_Vmax = pwm;
      max_freq_divider = freq_divider;     
      String result = String("New max. reached (V_in,A_in,V_out,A_out,Eff, Freq, duty cycle: ");
      result = result + voltage_input + String(" ") + current_input + String(" ");
      result = result + voltage_output + String(" ") + current_output + String(" ");
      if(voltage_input !=0.0 and current_input !=0.0) {
       result = result + (voltage_output*current_output)/(voltage_input*current_input)*100 + String("% ");
      } else result = result + String("N/A% ");
      result = result + freq_divider  + String(" ") + float(pwm)/freq_divider*100 + String("% ");
      Serial.println(result);
 
    }
 
    if (voltage_output > 26.0) { //approaching saturation
      String result = String("Maximum target voltage reached with: ");
      result = result + + freq_divider + String(" ") + pwm + String(" ") + voltage_output;
      Serial.println(result);
      while(true){}
    }
  
  }
 
}


void setup() {

  //if you take smaller baud, cannot send messages if interrupt timer is set to high frequencies
  Serial.begin(2000000); //max 10000 or maybe even 20000

  Serial.println("Starting the test");

  pinMode(D5, OUTPUT); // output pin for OCR0A
  // Set up the fast PWM up to 8Mhz output @D5
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(WGM02) | _BV(CS00);
  //to be done during the test
  OCR0A = 255; //freq. divider // 1--> 8MHz,1bit pwm, 3-->4Mhz,2bit pwm, 7-->2Mhz,3bit pwm, 15-->1Mhz,4bit pwm etc.
  OCR0B = 0;
  
  //test for the best configuration
  for(int freq_divider = 255; freq_divider>0; freq_divider--) {
    String result = String("Testing freq divider: ") + freq_dividera + String(" ");
    result = result + max_freq_divider + String(" ") + pwm_for_Vmax + String(" ") + Vmax;
    Serial.println(result);
    find_max(freq_divider);
    //delay((float(freq_divider)*255-245)/20); // delay x ms --> 1 s becomes 62500 due to change of the clock multiplier
    //delay(62500/20);
  }

  Serial.println("Test finished.");
  OCR0A = max_freq_divider;  
  OCR0B = pwm_for_Vmax; 
}


void loop() {

String result = String("Test results (max_freq_divider, pwn_for_Vmax, duty cyclw, Vmax): ");
result = result + max_freq_divider + String(" ") + pwm_for_Vmax  + String(" ");
result = result + float(pwm_for_Vmax)/max_freq_divider*100 + String("% ") + Vmax;
Serial.println(result);

delay(62500); // delay x ms --> 1 s becomes 62500 due to change of the clock multiplier

}
