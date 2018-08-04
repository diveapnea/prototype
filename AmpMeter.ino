// An Pin 13 ist eine LED angeschlossen, die auf den meisten Arduino Boards vorhanden ist
const int LED = 13;
const int decimation = 128; //4^x --> x bits of additional precision

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //max 10000 or maybe even 20000
  pinMode(LED, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  //current sensor ACS712T
  //-30A --> 0,5V
  //0A --> 2,5V
  //30A --> 4,5V
  
  //decimation (increases precision)
  float A7_val=0;
  float A6_val=0;
  for(int i=0;i<decimation;i++){
      // read the input on analog pin 7:
    A7_val = A7_val + analogRead(A7);
    A6_val = A6_val + analogRead(A6);
    delay(1);
  }
  A7_val = A7_val/decimation;
  A6_val = A6_val/decimation;
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):  
  //float voltage = A7_val * (5.0 / 1023.0);
  //convert voltage into current (0V-->-30A, 2,5V-->0A, 5V-->+30A: f(x)=12X-30
  //float current = (voltage - 2.5)*60/4;
  //combining voltage and current conversions AND approximation gives out: f_current(x) = 0.0586510264*A7_val-30
  
  float current = 0.05865*A7_val-30;
  // print out the value you read:
  String c_unit = "A";
  String result = current + c_unit;

  float voltage = A6_val * (5.0 / 1023.0);
  c_unit = "V";
  result = voltage + c_unit + " ; " + result;
  
  Serial.println(result);

  //digitalWrite(LED, HIGH); // LED anschalten
  //delay(50); // 50 ms warten
  //digitalWrite(LED, LOW); // LED ausschalten
  delay(1); // x ms warten



}
