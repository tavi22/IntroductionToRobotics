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
const int pinSW = 2;  // digital pin connected to switch output
const int pinX = A1;  // A1 - analog pin connected to X output
const int pinY = A0;  // A0 - analog pin connected to Y output

// segment idexes for each led
const byte aIndex = 0;
const byte bIndex = 1;
const byte cIndex = 2;
const byte dIndex = 3;
const byte eIndex = 4;
const byte fIndex = 5;
const byte gIndex = 6;
const byte dpIndex = 7;

// modify if you have common anode
bool commonAnode = false;
byte state = HIGH;

// led variables
volatile byte currentLed = 0;
unsigned long lastBlink = 0;
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
volatile byte swState = LOW;
int xValue = 0;
int yValue = 0;
const int lowerThreshold = 200;
const int upperThreshold = 800;
const int highMiddleThreshold = 600;
const int lowMiddleThreshold = 400;
volatile bool joyMoved = 0;

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

  pinMode(pinSW, INPUT_PULLUP);  // activate pull-up resistor on the push-button pin

  // Start the serial communication.
  // Serial.begin(9600);
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
        }
      }
    }
  }

  lastReading = reading;
}


// turn off all segments and move to dp
void resetSegments() {
  for (int i = 0; i < segSize; i++) {
    segmentStates[i] = LOW;
    trueSegmentStates[i] = LOW;
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

    // state 1: blink current led segment and move between segments with the joystick
    if (millis() - lastBlink >= blinkDelay) {
      lastBlink = millis();
      segmentStates[currentLed] = !segmentStates[currentLed];
    }

    // handle joystick movements
    if (xValue > upperThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold && joyMoved == 0) {
      move('r');  // move right
      joyMoved = 1;
    }
    if (yValue > upperThreshold && xValue < highMiddleThreshold && xValue > lowMiddleThreshold && joyMoved == 0) {
      move('u');  // move up
      joyMoved = 1;
    }
    if (xValue < lowerThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold && joyMoved == 0) {
      move('l');  // move left
      joyMoved = 1;
    }
    if (yValue < lowerThreshold && xValue < highMiddleThreshold && xValue > lowMiddleThreshold && joyMoved == 0) {
      move('d');  // move right
      joyMoved = 1;
    }
    if (xValue < highMiddleThreshold && xValue > lowMiddleThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      joyMoved = 0;
    }
  }

  else {
    
    // state 2: turn current led on/off by moving the joystick to right/left
    segmentStates[currentLed] = trueSegmentStates[currentLed];
    displaySegments(segments);

    if (xValue > upperThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      trueSegmentStates[currentLed] = HIGH;
    } else if (xValue < lowerThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      trueSegmentStates[currentLed] = LOW;
    }
  }
}

// handle joystick movements in state 1
// define all possible movements from each state in every direction
void move(char direction) {
  switch (direction) {
    case 'r':
      switch (currentLed) {
        case aIndex:
          currentLed = bIndex;
          segmentStates[aIndex] = trueSegmentStates[aIndex];
          break;
        case cIndex:
          currentLed = dpIndex;
          segmentStates[cIndex] = trueSegmentStates[cIndex];
          break;
        case dIndex:
          currentLed = cIndex;
          segmentStates[dIndex] = trueSegmentStates[dIndex];
          break;
        case eIndex:
          currentLed = cIndex;
          segmentStates[eIndex] = trueSegmentStates[eIndex];
          break;
        case fIndex:
          currentLed = bIndex;
          segmentStates[fIndex] = trueSegmentStates[fIndex];
          break;
        default:
          break;
      }
      break;
    case 'u':
      switch (currentLed) {
        case bIndex:
          currentLed = aIndex;
          segmentStates[bIndex] = trueSegmentStates[bIndex];
          break;
        case cIndex:
          currentLed = gIndex;
          segmentStates[cIndex] = trueSegmentStates[cIndex];
          break;
        case dIndex:
          currentLed = gIndex;
          segmentStates[dIndex] = trueSegmentStates[dIndex];
          break;
        case eIndex:
          currentLed = gIndex;
          segmentStates[eIndex] = trueSegmentStates[eIndex];
          break;
        case fIndex:
          currentLed = aIndex;
          segmentStates[fIndex] = trueSegmentStates[fIndex];
          break;
        case gIndex:
          currentLed = aIndex;
          segmentStates[gIndex] = trueSegmentStates[gIndex];
          break;
        default:
          break;
      }
      break;
    case 'l':
      switch (currentLed) {
        case aIndex:
          currentLed = fIndex;
          segmentStates[aIndex] = trueSegmentStates[aIndex];
          break;
        case bIndex:
          currentLed = fIndex;
          segmentStates[bIndex] = trueSegmentStates[bIndex];
          break;
        case cIndex:
          currentLed = eIndex;
          segmentStates[cIndex] = trueSegmentStates[cIndex];
          break;
        case dIndex:
          currentLed = eIndex;
          segmentStates[dIndex] = trueSegmentStates[dIndex];
          break;
        case dpIndex:
          currentLed = cIndex;
          segmentStates[dpIndex] = trueSegmentStates[dpIndex];
          break;
        default:
          break;
      }
      break;
    case 'd':
      switch (currentLed) {
        case aIndex:
          currentLed = gIndex;
          segmentStates[aIndex] = trueSegmentStates[aIndex];
          break;
        case bIndex:
          currentLed = gIndex;
          segmentStates[bIndex] = trueSegmentStates[bIndex];
          break;
        case cIndex:
          currentLed = dIndex;
          segmentStates[cIndex] = trueSegmentStates[cIndex];
          break;
        case eIndex:
          currentLed = dIndex;
          segmentStates[eIndex] = trueSegmentStates[eIndex];
          break;
        case fIndex:
          currentLed = gIndex;
          segmentStates[fIndex] = trueSegmentStates[fIndex];
          break;
        case gIndex:
          currentLed = dIndex;
          segmentStates[gIndex] = trueSegmentStates[gIndex];
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}