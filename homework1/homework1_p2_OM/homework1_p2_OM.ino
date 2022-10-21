//  Fading an LED with the potentiometer

const int potPin = A0;
const int ledPin = 5;

// global variables are initialized to 0 by default
int potValue, ledValue;

void setup() {
  pinMode(potPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  potValue = analogRead(potPin);
  // ledValue = potValue / 4;   // easiest working way (not very accurate)
  // ledValue = map(potValue, 0, 1023, 0, 255);   // predefined map function
  ledValue = mapValue(potValue, 1023, 255);   // manual mapping function using percentages
  analogWrite(ledPin, ledValue);
}

int mapValue(int val, int firstMax, int secondMax) {
  float percentage = (float) val / (float) firstMax;
  float mappedValue = (float) secondMax * percentage;

  return round(mappedValue);
}



