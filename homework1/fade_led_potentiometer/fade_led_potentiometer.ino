//  Fading an LED with the potentiometer

const int potPin = A0;
const int ledPin = 5;
const int maxAnlogValue = 1023;
const int maxDiodeValue = 255;

// global variables are initialized to 0 by default
int potValue, ledValue;

void setup() {
  pinMode(potPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  potValue = analogRead(potPin);
  // ledValue = potValue / 4;   // easiest working way (not very accurate)
  // ledValue = map(potValue, 0, maxAnalogValue, 0, maxDiodeValue);   // predefined map function
  ledValue = mapValue(potValue, maxAnalogValue, maxDiodeValue);   // manual mapping function using percentages
  analogWrite(ledPin, ledValue);
}

int mapValue(int val, int firstMax, int secondMax) {
  float percentage = (float) val / (float) firstMax;
  float mappedValue = (float) secondMax * percentage;

  return round(mappedValue);
}



