// declare all the segments pins
const int pinA = 4;
const int pinB = 5;
const int pinC = 6;
const int pinD = 7;
const int pinE = 8;
const int pinF = 9;
const int pinG = 10;
const int pinDP = 11;

// joystick variables
const int pinSW = 2; // digital pin connected to switch output
const int pinX = A1; // A1 - analog pin connected to X output
const int pinY = A0; // A0 - analog pin connected to Y output

// segment idexes for each led
byte aIndex = 0;
byte bIndex = 1;
byte cIndex = 2;
byte dIndex = 3;
byte eIndex = 4;
byte fIndex = 5;
byte gIndex = 6;
byte dpIndex = 7;

// modify if you have common anode
bool commonAnode = false; 
byte state = HIGH;

// led variables
volatile byte currentLed = 0;
unsigned long lastBlink = 0;
unsigned long timer = 0;
const int blinkDelay = 500;

volatile byte currentState = 1;

// 7 digit display variables
const int segSize = 8;

int segments[segSize] = { 
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

byte segmentStates[segSize] = {
  LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
};  // the array which prints the leds

byte trueSegmentStates[segSize] = {
  LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
};  // copy of the array from above, which contains the true led values

// joystick variables
byte swState = LOW;
int xValue = 0;
int yValue = 0;
const int lowerThreshold = 200;
const int upperThreshold = 800;
const int highMiddleThreshold = 600;
const int lowMiddleThreshold = 400;

// button variables
const int debounceDelay = 50;
unsigned long int lastDebounce = 0;
const int longPressDelay = 1000;
volatile byte lastReading = LOW;

void setup() {
  // initialize all the pins
  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }
  if (commonAnode == true) {
    state = !state;
  }

  pinMode(pinSW, INPUT_PULLUP); // activate pull-up resistor on the push-button pin
  
  // Start the serial communication.
  Serial.begin(9600);
}

void loop() {
  displaySegments(segments);  
  checkState(currentState, currentLed);

  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  volatile byte reading = digitalRead(pinSW);

  if (lastReading != reading) {
    lastDebounce = millis();
  }
  // handle long button press
  else if ((millis() - lastDebounce) >= longPressDelay && reading == LOW && currentState == 1) {
    resetSegments();
  }

  if ((millis() - lastDebounce) >= debounceDelay) {
    if (swState != reading) {
      swState = reading;

      if (!swState) {
        if (currentState == 1) {
          currentState = 2;
        } else {
          currentState = 1;
          segmentStates[currentLed] = trueSegmentStates[currentLed];
          incrementCurrentLed();
        }
      }
    }
  }
  
  lastReading = reading;
}


void resetSegments() {
 for (int i = 0; i < segSize; i++) {
   segmentStates[i] = LOW;
 }

 currentLed = dpIndex;
}

void displaySegments(int segments[]) {
  for (int i = 0; i < segSize; i++) {
    digitalWrite(segments[i], segmentStates[i]);
  }

  // digitalWrite(segments[segSize-1], dpState);
}

void checkState(byte currentState, byte currentLed) {
  if (currentState == 1) {
    
    if (millis() - lastBlink >= blinkDelay) {
      lastBlink = millis();
      segmentStates[currentLed] = !segmentStates[currentLed];
    }

    if (xValue > upperThreshold) {
      move('r');  // move right
    }
    if (yValue > upperThreshold) {
      move('u');  // move up
    }
    if (xValue < lowerThreshold) {
      move('l');  // move left
    }
    if (yValue < lowerThreshold) {
      move('d');  // move right
    }
  }

  else {
    segmentStates[currentLed] = trueSegmentStates[currentLed];
    displaySegments(segments);  

    if (xValue > upperThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      trueSegmentStates[currentLed] = HIGH;
    } else if (xValue < lowerThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      trueSegmentStates[currentLed] = LOW;
    }
  }
}

void incrementCurrentLed() {
  if (currentLed == 7) {
    currentLed = 0;
  }
  else currentLed++;
}

void move(char direction) {
  switch(direction) {
    case 'r':
      Serial.println("Miscare dreapta");
      break;
    case 'u':
      break;
    case 'l':
      break;
    case 'd':
      break;
    default:
      break;
  }
}

