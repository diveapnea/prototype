
void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000); //max 1000000
  pinMode(D3, OUTPUT);

}

void loop() {

  // put your main code here, to run repeatedly:
  
  //current sensor ACS712T
  //-30A --> 0,5V
  //0A --> 2,5V
  //30A --> 4,5V
  
  //combining voltage and current conversions AND approximation gives out: f_current(x) = 0.0586510264*A7_val-30
  
  float current = 0.05865*analogRead(A7)-30;
  float voltage = analogRead(A6) * (5.0 / 1023.0);

  Serial.println(current,1); //low precision for high speed. 0.1A is the best we can get here
  Serial.println(voltage);

}
