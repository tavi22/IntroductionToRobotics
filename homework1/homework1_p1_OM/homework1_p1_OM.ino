// Blink without delay

const int ledPin = 13;
int ledState = LOW;   // could be bool

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;
const long interval = 1000;   // interval at which to blink (milliseconds)

void setup() {

  pinMode(ledPin, OUTPUT);

}

void loop() {

  unsigned long currentMillis = millis();

  // check if the interval has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // turn the led on/off
    if (!ledState) {
    ledState = HIGH;

  } else {
    ledState = LOW;

  }
  digitalWrite(ledPin, ledState);

  }
}