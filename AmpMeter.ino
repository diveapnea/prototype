// An Pin 13 ist eine LED angeschlossen, die auf den meisten Arduino Boards vorhanden ist
const int LED = 13;
const int decimation = 64; //4^x --> x bits of additional precision

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //max 10000 or maybe even 20000
  pinMode(LED, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  
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

  //current sensor ACS712
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):  
  //float voltage = A7_val * (5.0 / 1023.0);
  //convert voltage into current (0.5V-->-30A, 2,5V-->0A, 4.5V-->+30A: f_current(x)=15x-37.5
  //f_current(A7_val) =  (15 * A7_val * 5.0 / 1023.0) - 37.5 --> 0.073313 * A7_val - 37.5
  
  //convert voltage into current (1.5V-->-5A, 2,5V-->0A, 3.5V-->+5A: f_current(x)=5x-12.5  
  //f_current(A7_val) =  (5 * A7_val * 5.0 / 1023.0) - 12.5 --> 0.024438 * A7_val - 12.5
  
  float current = 0.024438*A7_val-12.5;
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
