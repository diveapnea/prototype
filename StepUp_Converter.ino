const int D3 = 3;
uint8_t pwm = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(D3, OUTPUT);
  analogWrite(D3, pwm); //PWM on D3  analogWrite values from 0 to 255  
}


void loop() {


  float voltage = analogRead(A6) * (30.0*0.9384/ 1023.0); //voltage divider used to sense 5V at 30V real//0.9384 is the calibration value
  if (voltage < 12.5 & pwm <230) pwm = pwm+1; //do not go to 100% PWM, which does not move anything
  else if (pwm > 0) pwm = pwm-1; //underflow can occur
  analogWrite(D3, pwm); //PWM on D3  analogWrite values from 0 to 255  

}
