#define F_CPU 8000000
int pwm = 1;
int potinput = A2;
int feedbackinput = A3;
int potinputval;
int feedbackinputval;
int pwmval;

void setup() {
  TCCR0A = 2 << COM0A0 | 2 << COM0B0 | 3 << WGM00;
  TCCR0B = 0 << WGM02 | 1 << CS00;
  TCCR1 = 0 << PWM1A | 0 << COM1A0 | 1 << CS10;
  GTCCR = 1 << PWM1B | 2 << COM1B0;
  pinMode(pwm, OUTPUT);
  pinMode(potinput, INPUT);
  pinMode(feedbackinput, INPUT);
  digitalWrite(pwm, LOW);
  pwmval = 0;
}

void loop() {
  potinputval = analogRead(potinput);
  potinputval = map(potinputval, 1023, 0, 255, 0);
  feedbackinputval = analogRead(feedbackinput);
  feedbackinputval = map(feedbackinputval, 1023, 0, 255, 0);
  while (potinputval > feedbackinputval) {
    if (pwmval == 230) {
      potinputval = analogRead(potinput);
      potinputval = map(potinputval, 1023, 0, 255, 0);
      feedbackinputval = analogRead(feedbackinput);
      feedbackinputval = map(feedbackinputval, 1023, 0, 255, 0);
    }
    else {
      pwmval = pwmval + 1;
      analogWrite(pwm, pwmval);
      potinputval = analogRead(potinput);
      potinputval = map(potinputval, 1023, 0, 255, 0);
      feedbackinputval = analogRead(feedbackinput);
      feedbackinputval = map(feedbackinputval, 1023, 0, 255, 0);
    }
  }
  while (potinputval < feedbackinputval) {
    if (pwmval == 0) {
      potinputval = analogRead(potinput);
      potinputval = map(potinputval, 1023, 0, 255, 0);
      feedbackinputval = analogRead(feedbackinput);
      feedbackinputval = map(feedbackinputval, 1023, 0, 255, 0);
    }
    else {
      pwmval = pwmval - 1;
      analogWrite(pwm, pwmval);
      potinputval = analogRead(potinput);
      potinputval = map(potinputval, 1023, 0, 255, 0);
      feedbackinputval = analogRead(feedbackinput);
      feedbackinputval = map(feedbackinputval, 1023, 0, 255, 0);
    }
  }
}
