/*
 * Pin 5 and 6 can go up tp 62,5kHz PWM
 * But the delay function becomes delay(62500) for 1 second
 * 0x01 ->62500khz
 * 0x02 -->7800hz
 * 0x03 -->980Hz (default for pin 5/6)
 * 0x04 -->250hz
 * 0x05 -->61hz
 * 
 * For pin 3,9,10,11 same thing with 31,3kHz max frequency
 * For pin3,9,10,11 counters would be TCCR1B and TCCR2B
 */

const int D5 = 5;
uint8_t pwm = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(D5, OUTPUT);
  analogWrite(D5, pwm); //PWM on D5  analogWrite values from 0 to 255  
  TCCR0B = TCCR0B & 0b11111000 | 0x01; //Pin5 0x01 --> 62,5kHz pwm
    
}


void loop() {


  float voltage = analogRead(A6) * (30.0*0.9384/ 1023.0); //voltage divider used to sense 5V at 30V real//0.9384 is the calibration value
  if (voltage < 12.5 & pwm <144) pwm = pwm+1; //do not go to 100% PWM, which does not move anything
  else if (pwm > 0) pwm = pwm-1; //underflow can occur
  analogWrite(D5, pwm); //PWM on D5  analogWrite values from 0 to 255  

}
