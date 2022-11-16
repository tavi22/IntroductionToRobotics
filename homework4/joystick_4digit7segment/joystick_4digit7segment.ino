// control a 4 digit 7-segment display with a joystick

// 4 digit 7-segment variables
const int latchPin = 11; // STCP to 12 on Shift Register
const int clockPin = 10; // SHCP to 11 on Shift Register
const int dataPin = 12; // DS to 14 on Shift Register

const int segD1 = 7;
const int segD2 = 6;
const int segD3 = 5;
const int segD4 = 4;

const byte regSize = 8;

int displayDigits[] = {
  segD1, segD2, segD3, segD4
};
const int displayCount = 4;
const int encodingsNumber = 16;

int byteEncodings[encodingsNumber] = {
//A B C D E F G DP 
  B11111100, // 0 
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B11101110, // A
  B00111110, // b
  B10011100, // C
  B01111010, // d
  B10011110, // E
  B10001110  // F
};

// actual displayed characters on the display (value 0 for each in the beginning)
int displayCharacters[displayCount] = {
  byteEncodings[0], byteEncodings[0], byteEncodings[0], byteEncodings[0]
};

// joystick variables
const int pinSW = 2;  // digital pin connected to switch output
const int pinX = A1;  // A1 - analog pin connected to X output
const int pinY = A0;  // A0 - analog pin connected to Y output

byte swState = LOW;
int xValue = 0;
int yValue = 0;
const int lowerThreshold = 200;
const int upperThreshold = 800;
const int highMiddleThreshold = 600;
const int lowMiddleThreshold = 400;
bool joyMoved = 0;

byte currentDisplay = 3;  // rightmost display
byte currentState = 1;
unsigned long lastBlink = 0;
const int blinkDelay = 500;

// button variables
const int debounceDelay = 50;
unsigned long int lastDebounce = 0;
unsigned long int releasedTime = 0;
const int longPressDelay = 1000;
volatile byte lastReading = LOW;

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pinSW, INPUT_PULLUP);  // activate pull-up resistor on the push-button pin


  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  Serial.begin(9600);
}

void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  byte reading = digitalRead(pinSW);

  writeCharacters(displayCharacters);

  // handle button click
  if (lastReading != reading) {
      lastDebounce = millis();

    }
  else if ((millis() - lastDebounce) >= longPressDelay && reading == LOW && currentState == 2) {
    resetDisplay();
  }

  if ((millis() - lastDebounce) >= debounceDelay) {
    if (swState != reading) {
      swState = reading;

      if (!swState) {
        if (currentState == 1) {
          currentState = 2;
          turnOnDP();
        } else {
          currentState = 1;
        }
      }
    }
  }

  lastReading = reading;

  // handle each state
  if (currentState == 1) {
    blinkDP();
    handleDisplayChange();

  } else {
    handleCharacterChange();
  }
  
}

void blinkDP() {
  if (millis() - lastBlink >= blinkDelay) {
    lastBlink = millis();

    if (displayCharacters[currentDisplay] % 2 == 0) {
      displayCharacters[currentDisplay]++;
    } else {
      displayCharacters[currentDisplay]--;
    }
  }
}

// change which display to modify with x axis
void handleDisplayChange() {
  if (xValue > upperThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold && joyMoved == 0) {
    if (currentDisplay < 3) {
      turnOffDP();
      currentDisplay++;
    }
    joyMoved++;

  } else if (xValue < lowerThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold && joyMoved == 0) {
    if (currentDisplay > 0) {
      turnOffDP();
      currentDisplay--;
    }
    joyMoved++;

  } else if (xValue < highMiddleThreshold && xValue > lowMiddleThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      joyMoved = 0;
    }
}

// modify the characters with y axis
void handleCharacterChange() {
  int lastValue = lookUpCharacter(displayCharacters[currentDisplay] - 1);

  if (yValue > upperThreshold && xValue < highMiddleThreshold && xValue > lowMiddleThreshold && joyMoved == 0) {
    increaseDisplayCharacter(lastValue);
    turnOnDP();
    joyMoved++;

  } else if (yValue < lowerThreshold && xValue < highMiddleThreshold && xValue > lowMiddleThreshold && joyMoved == 0) {
    decreaseDisplayCharacter(lastValue);
    turnOnDP();
    joyMoved++;

  } else if (xValue < highMiddleThreshold && xValue > lowMiddleThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      joyMoved = 0;
    }
}

void writeReg(int character) {
    // ST_CP LOW to keep LEDs from changing while reading serial data
    digitalWrite(latchPin, LOW);
    // Shift out the bits
    shiftOut(dataPin, clockPin, MSBFIRST, character);
    /* ST_CP on Rising to move the data from shift register
     * to storage register, making the bits available for output.
     */
    digitalWrite(latchPin, HIGH);
}

void activateDisplay(int displayNumber) { 
  // first, disable all the display digits
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  // then enable only the digit you want to use now
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeCharacters(int displayCharacters[]) {
  int currentIndex = displayCount - 1;
  int displayDigit = 0;
  int lastCharacter = 0;

  while (currentIndex >= 0) {
    // get the last character of the display
    lastCharacter = displayCharacters[currentIndex];
    // send the number to the display
    writeReg(lastCharacter);
    // enable only the display digit for that 
    activateDisplay(displayDigit);

    delay(5);
    // increment the display digit
    displayDigit++;
    // move the index
    currentIndex--;
  }
}

void resetDisplay() {
  for (int i = 0; i < displayCount; i++) {
    displayCharacters[i] = byteEncodings[0];
  }
  currentState = 1;
  currentDisplay = 3;
}

void turnOffDP() {
  if (displayCharacters[currentDisplay] % 2 != 0) {
        displayCharacters[currentDisplay]--;
      }
}

void turnOnDP() {
  if (displayCharacters[currentDisplay] % 2 == 0) {
        displayCharacters[currentDisplay]++;
      }
}

int lookUpCharacter(int character) {
  for (int i = 0; i < encodingsNumber; i++) {
    if (byteEncodings[i] == character) {
      return i;
    }
  }
  return -1;
}

void increaseDisplayCharacter(int lastValue) {
  if (lastValue == 15) {
    displayCharacters[currentDisplay] = byteEncodings[0];
  } else
  displayCharacters[currentDisplay] = byteEncodings[lastValue + 1];
}

void decreaseDisplayCharacter(int lastValue) {
  if (lastValue == 0) {
    displayCharacters[currentDisplay] = byteEncodings[15];
  } else
  displayCharacters[currentDisplay] = byteEncodings[lastValue - 1];
}