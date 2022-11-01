// button variables
const int buttonPin = 2;
volatile byte buttonState;
const int buttonPressDelay = 8000;
volatile byte lastButtonState = LOW;

// buzzer variables
const int buzzerPin = 3;
const int buzzerGreenTone = 450;
const int buzerBlinkingGreenTone = 650;
unsigned long buzzerTimer = 0;

// pedestrians lights variables
const int greenLightPedestriansPin = 7;
byte greenLightPedestriansState = LOW;
const int greenLightDuration = 8000;
const int resetDelay = 12000;

const int redLightPedestriansPin = 8; 
byte redLightPedestriansState = HIGH;

// cars lights variables
const int greenLightVehiclesPin = 11;
byte greenLightVehiclesState = HIGH;

const int yellowLightVehiclesPin = 12;
byte yellowLightVehiclesState = LOW;
const int yellowLightDuration = 3000;

const int redLightVehiclesPin = 13; 
byte redLightVehiclesState = LOW;

// debounce variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const int blinkDelay = 500;
const int blinkFaster = 300;
volatile int currentState = 1;
unsigned long timer = 0;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(greenLightPedestriansPin, OUTPUT);
  pinMode(redLightPedestriansPin, OUTPUT);
  pinMode(greenLightVehiclesPin, OUTPUT);
  pinMode(yellowLightVehiclesPin, OUTPUT);
  pinMode(redLightVehiclesPin, OUTPUT);
  // attachInterrupt(digitalPinToInterrupt(buttonPin), isr, FALLING);

  // Serial.begin(9600);

}

// turn the lights on/off
void handleLights() {
  digitalWrite(greenLightPedestriansPin, greenLightPedestriansState);
  digitalWrite(redLightPedestriansPin, redLightPedestriansState);
  digitalWrite(greenLightVehiclesPin, greenLightVehiclesState);
  digitalWrite(yellowLightVehiclesPin, yellowLightVehiclesState);
  digitalWrite(redLightVehiclesPin, redLightVehiclesState);
}

// first state: red for pedestrians, green for cars
void baseState() {
  greenLightPedestriansState = LOW;
  redLightPedestriansState = HIGH;

  greenLightVehiclesState = HIGH;
  yellowLightVehiclesState = LOW;
  redLightVehiclesState = LOW;

  currentState = 1;
  buzzerTimer = 0;
  timer = 0;

}

// second state: red for pedestrians, yellow for cars
void buttonPressedState() {
  greenLightVehiclesState = LOW;
  yellowLightVehiclesState = HIGH;
  currentState = 3;
  timer = millis();
}

// third state: green for pedestrians, red for cars, buzzer beeping
void pedCrossState() {
  redLightVehiclesState = HIGH;
  yellowLightVehiclesState = LOW;
  greenLightVehiclesState = LOW;
  
  greenLightPedestriansState = HIGH;
  redLightPedestriansState = LOW;
  
  currentState = 4;
  timer = millis();
}

void startBuzzer() {
  if ((millis() - buzzerTimer) > blinkDelay) {
    tone(buzzerPin, buzzerGreenTone, blinkFaster);
    buzzerTimer = millis();
  }
}

// fourth state: blinking green for pedestrians, red for cars, buzzer beeping faster
void blinkingGreenState() {
  if ((millis() - buzzerTimer) > blinkFaster) {
  tone(buzzerPin, buzerBlinkingGreenTone, blinkFaster);
  greenLightPedestriansState = !greenLightPedestriansState;
  buzzerTimer = millis();
  }
}

void loop() {

  handleLights();

  byte reading = digitalRead(buttonPin);
  
  if (reading != lastButtonState && currentState == 1) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (!buttonState && currentState == 1) {
        timer = millis();
        currentState = 2;
      }
    }
  }

  // after button is pressed wait 8 seconds and initiate second state
  if ((millis() - timer) >= buttonPressDelay && currentState == 2) {
    buttonPressedState();
  }

  // yellow light stays on for 3 seconds and initiate third state
  if ((millis() - timer) >= yellowLightDuration && currentState == 3) {
    pedCrossState();
  }

  // beep the buzzer while the green light is on for pedestrians
  if ((millis() - timer) < greenLightDuration && currentState == 4) {
    startBuzzer();
  }

  // after 8 seconds initiate the fourth and final state
  if ((millis() - timer) >= greenLightDuration && currentState == 4 && (millis() - timer) < resetDelay) {
    blinkingGreenState();
  }

  // after 12 seconds return to first state
  if ((millis() - timer) >= resetDelay && currentState == 4) {
    baseState();
  }

  lastButtonState = reading;
}

