//Atmega 328 tested
/* 
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
const int D3 = 3;
uint8_t pwm = 0;
uint8_t pwm_max = 0;

void setup() {
  //Serial.begin(9600); //max 10000 or maybe even 20000
  // put your setup code here, to run once:
 /*
  analogWrite(D5, pwm); //PWM on D5  analogWrite values from 0 to 255  
  TCCR0B = TCCR0B & 0b11111000 | 0x03; //Pin5 0x01 --> 62,5kHz pwm
  pwm_max = 255 * 0.9;
*/
  /*
  pinMode(D3, OUTPUT); // output pin for OCR2B
  // Set up the 250KHz output @ D3
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = 63; //freq. divider // 1--> 8MHz,1bit pwm, 3-->4Mhz,2bit pwm, 7-->2Mhz,3bit pwm, 15-->1Mhz,4bit pwm etc.
  OCR2B = 32; //50% duty cycle. 0..to 63
*/

  pinMode(D5, OUTPUT); // output pin for OCR0A
  // Set up the fast PWM up to 8Mhz output @D5
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(WGM02) | _BV(CS00);
  OCR0A = 255; //freq. divider // 1--> 8MHz,1bit pwm, 3-->4Mhz,2bit pwm, 7-->2Mhz,3bit pwm, 15-->1Mhz,4bit pwm etc.
  OCR0B = OCR0A/2; //duty cycle (0 means 50% for 1 bit)

  pwm_max= OCR0A * 0.8; //if standard 62,5 kHz osc. used, OCR0A is zero. So replace with 255


}


void loop() {

  int raw_analog_reading = analogRead(A6); //remember ADC is 10bit
  
  //I/O over voltage protection
  if(raw_analog_reading > 1020){ //approaching saturation
    pwm = 0;
    analogWrite(D5, pwm);
  }
  
  float voltage = raw_analog_reading * (30.0*0.9961/ 1023.0); //voltage divider used to sense 5V at 30V real//0.9961 is the calibration value

  if (voltage < 13.0 & pwm < pwm_max) pwm = pwm+1; //do not go to 100% PWM, which does not move anything. 80-90% seems to be a golden value
  else if (pwm > 0) pwm = pwm-1; //underflow can occur

  // fly away protection. Seems not to make sense but it works. Checks whether a small PWM change leverages the voltage or not
  if(pwm > raw_analog_reading) pwm = 0;
  
  
  //this command is same as assigning a value to OCR0B
  analogWrite(D5, pwm); //PWM on D5  analogWrite values from 0 to 255 or OCR0A if non-zero
  
  delay(62500/2000); // delay x ms --> 1 s becomes 62500 due to change of the clock multiplier
  
  //String result = raw_analog_reading + String(" ") + pwm;
  //Serial.println(result);

}
