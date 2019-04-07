
void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000); //max 1000000
}

void loop() {

  // put your main code here, to run repeatedly:
  
 // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):  
  //float voltage = A7_val * (5.0 / 1023.0);
  //convert voltage into current (0.5V-->-30A, 2,5V-->0A, 4.5V-->+30A: f_current(x)=15x-37.5
  //f_current(A7_val) =  (15 * A7_val * 5.0 / 1023.0) - 37.5 --> 0.073313 * A7_val - 37.5
  
  //convert voltage into current (1.5V-->-5A, 2,5V-->0A, 3.5V-->+5A: f_current(x)=5x-12.5  
  //f_current(A7_val) =  (5 * A7_val * 5.0 / 1023.0) - 12.5 --> 0.024438 * A7_val - 12.5

      
  float current = 0.024438*analogRead(A7)-12.5;
  float voltage = analogRead(A6) * (5.0 / 1023.0);

  Serial.println(current,1); //low precision for high speed. 0.1A is the best we can get here
  Serial.println(voltage);

}
