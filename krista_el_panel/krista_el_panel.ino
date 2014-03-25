int controlPin = 7;
int switchPin = 12;

void setup() {
  pinMode(controlPin, OUTPUT);
  pinMode(switchPin, INPUT);
}

void loop() {
  int switchVal = digitalRead(switchPin);
  digitalWrite(controlPin, switchVal);
}
