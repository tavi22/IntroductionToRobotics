#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>

// LCD variables
const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(RS,enable,d4,d5,d6,d7);

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
const int debounceDelay = 50;
unsigned long int lastDebounce = 0;
byte lastReading = LOW;
byte reading = LOW;

// matrix variables
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin,1);  //DIN, CLK, LOAD, No.

// auxiliary matrix variables
byte matrixBrightness = 2;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
const byte moveInterval = 100;
unsigned long long lastMoved = 0;
bool matrixChanged = true;
byte xFood = random(8);
byte yFood = random(8);
byte xLastFood = 0;
byte yLastFood = 0;
unsigned long previousMillis = 0;
const long interval = 250;
int lives = 4;
int health = lives;

byte matrix[matrixSize][matrixSize] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

// auxiliary menu variables
const byte menuLength = 5;
const byte submenuLength = 7;
const long delayPeriod = 2000;
byte currentMenu = 0;


String menuItems[menuLength] = {
  "Start           ",
  "Leaderboard",
  "Settings",
  "About",
  "How to play"  
};

String settingsSubmenu[submenuLength] = {
  "Back",
  "Name",
  "Difficulty",
  "LCD contrast",
  "LCD brightness",
  "Matrix bright.",
  "Audio"
};

// state 0 is when the menu is displayed and state 1 means the game has started
byte state = 0;
// current menu option selected
byte menuCursor = 0;
byte lastCursor = 0;
// which menu items are displayed
byte displayState = 0;

// special LCD displays
byte heart1[] = {
  0x00,
  0x00,
  0x0A,
  0x15,
  0x11,
  0x0A,
  0x04,
  0x00
};

byte heart2[] = {
  0x00,
  0x00,
  0x0A,
  0x1F,
  0x1F,
  0x0E,
  0x04,
  0x00
};

struct Player {
  int score;
  char name[10];
};

Player currentPlayer = {0, "Newbie"};
// const int structSize = 8;


void setup() {
  pinMode(pinSW, INPUT_PULLUP);  // activate pull-up resistor on the push-button pin

  lcd.begin(16,2);
  lcd.createChar(1, heart1);
  lcd.createChar(2, heart2);

  Serial.begin(9600);
  // the zero refers to the MAX7219 number, it iszero for 1 chip
  lc.shutdown(0, false);  // turn off power saving,enables display
  lc.setIntensity(0, matrixBrightness);  // sets brightness(0~15 possiblevalues)
  lc.clearDisplay(0);  // clear screen
  matrix[xPos][yPos] = 1;
  matrix[xFood][yFood] = 1;
  
// display welcome message for 2 seconds before starting the application
  lcd.begin(16, 2);
  lcd.print("Interesting Name");
  lcd.setCursor(4, 1);
  lcd.print("Welcome!");
  delay(delayPeriod);
  lcd.clear();

  // Player plr = {0, "Unknown"};
  // EEPROM.put(0, plr);
  Player plr;
  EEPROM.get(0, plr);
  Serial.println(plr.name);
  Serial.println(plr.score);
}

void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  reading = digitalRead(pinSW);
  
  if (state == 0) {
    switch (currentMenu) {
      case 0:
        displayMenu();
        break;
      case 1:
        displayLeaderboard();
        break;
      case 2:
        displaySettings();
        break;
      case 3:
        displayAbout();
        break;
      case 4:
        displayHowTo();
        break;
    }
  } else {
    play();
    displayGameUI();
  }
}

void displayMenu() {
  int n = 0;
  handleJoystickYaxis(4, 3);
  handleJoystickPress();
  
  for (int i = 0; i < 2; i++) {
    int j = i + displayState;
    if (j == menuCursor) {
      lcd.setCursor(0, n);
      lcd.print("*");
      lcd.print(menuItems[j]);
      n++;
    } else {
      lcd.setCursor(0, n);
      lcd.print(" ");
      lcd.print(menuItems[j]);
      n++;
    }
  }
}

void displayGameUI() {
  // to do
  lcd.setCursor(0, 0);
  lcd.print("  Awesome Game");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(currentPlayer.score);
  lcd.print("    ");
  int healthCopy = health;
  for (int i = 0; i < lives; i++) {
    if (healthCopy > 0) {
      lcd.write(2);
      healthCopy--;
    } else {
      lcd.write(1);
    }
  }
  // quit the game by pressing the joystick (testing purposes)
  handleJoystickPress();
}

// game process
void play() {

  // food led blinking
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    if (!matrix[xFood][yFood]) {
      matrix[xFood][yFood] = 1;
      lc.setLed(0, xFood, yFood, 1);
    } else {
      matrix[xFood][yFood] = 0;
      lc.setLed(0, xFood, yFood, 0);
    }
  }

  // player movement
  if(millis() - lastMoved > moveInterval) {
    // game logic
    updatePositions();
    lastMoved = millis();
  }
  if(matrixChanged == true) {
    // matrix display logic
    updateMatrix();
    matrixChanged = false;
  }

  // eat the food => increase score => generate new food
  if (xPos == xFood && yPos == yFood) {
    currentPlayer.score++;
    generateFood();
  }
}

void generateFood() {
  xFood = random(8);
  yFood = random(8);
  matrix[xFood][yFood] = 1;
  matrixChanged = true;
}

void updateMatrix() {
  for(int row = 0; row < matrixSize; row++) {
    for(int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

void updatePositions() {
  xValue =analogRead(pinX);
  yValue =analogRead(pinY);

  xLastPos = xPos;
  yLastPos = yPos;
  
  if(xValue < lowerThreshold) {
    if(xPos < matrixSize - 1) {
      xPos++;
    }
    else {
      xPos = 0;
    }
  }

  if(xValue > upperThreshold) {
    if(xPos > 0) {
      xPos--;
    }
    else {
      xPos = matrixSize - 1;
    }
  }
  
  if(yValue > upperThreshold) {
    if(yPos < matrixSize - 1) {
      yPos++;
    }
    else {
      yPos = 0;
    }
  }
  
  if(yValue < lowerThreshold) {
    if(yPos > 0) {
      yPos--;
    }
    else {
      yPos = matrixSize - 1;
    }
  }
  
  if(xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 1;
  }
}

// called when a game ends
void stop() {
  Player plr;
  EEPROM.get(0, plr);

  if (currentPlayer.score > plr.score) {
    EEPROM.put(0, currentPlayer);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("!HIGHSCORE!");
    lcd.setCursor(0, 1);
    lcd.print("Leaderboard # 1");
    delay(delayPeriod);
    lcd.clear();
  }
}

// handle joystick movement for menu
void handleJoystickYaxis(byte maxCursor, byte maxState) {

  // menu items logic is to always see the next available option on the display
  if (yValue > upperThreshold && xValue < highMiddleThreshold && xValue > lowMiddleThreshold && joyMoved == 0) {
    if (menuCursor != 0) {
      menuCursor--;
    } else menuCursor = maxCursor;
    if (displayState != 0 && displayState != maxState || displayState == maxState && menuCursor == maxState) {
      displayState--;
    } else if (displayState == 0 && menuCursor == 0) {
      displayState = 0;
    }
    else displayState = maxState;

    joyMoved++;
    lcd.clear();

  } else if (yValue < lowerThreshold && xValue < highMiddleThreshold && xValue > lowMiddleThreshold && joyMoved == 0) {
    if (menuCursor != maxCursor) {
      menuCursor++;
    } else menuCursor = 0;
    if (displayState != maxState) {
      displayState++;    
    } else if (displayState == maxState && menuCursor == maxCursor) {
      displayState = maxState;
    }
    else displayState = 0;

    joyMoved++;
    lcd.clear();

  } else if (xValue < highMiddleThreshold && xValue > lowMiddleThreshold && yValue < highMiddleThreshold && yValue > lowMiddleThreshold) {
      joyMoved = 0;
    }
}

// handle joystick press
void handleJoystickPress() {
  if (lastReading != reading) {
      lastDebounce = millis();
  }

  if ((millis() - lastDebounce) >= debounceDelay) {
    if (swState != reading) {
      swState = reading;

      if (!swState) {
        lcd.clear();
        lcd.clear();

        if (menuCursor == 0 && currentMenu == 0) {
          if (state == 0) {
            state = 1;
          } else {
            state = 0;              
            stop();
          }
        } else {
          currentMenu = menuCursor;
          menuCursor = 0;
          displayState = 0;
        }
      }
    }
  }

  lastReading = reading;
}

void displayLeaderboard() {
  handleJoystickPress();
  Player plr;
  EEPROM.get(0, plr);
  lcd.setCursor(0, 0);
  lcd.print("> Highscores");
  lcd.setCursor(0, 1);
  lcd.print("#1 ");
  lcd.print(plr.name);
  lcd.print(": ");
  lcd.print(plr.score);
}

void displaySettings() {
  int n = 0;
  handleJoystickYaxis(6, 5);
  handleJoystickPress();
  
  for (int i = 0; i < 2; i++) {
    int j = i + displayState;
    if (j == menuCursor) {
      lcd.setCursor(0, n);
      lcd.print("*");
      lcd.print(settingsSubmenu[j]);
      n++;
    } else {
      lcd.setCursor(0, n);
      lcd.print(" ");
      lcd.print(settingsSubmenu[j]);
      n++;
    }
  }
}

void displayAbout() {
  handleJoystickPress();
  lcd.setCursor(0, 0);
  lcd.print("@UniBuc Robotics");
  lcd.setCursor(0, 1);
  lcd.print("bit.ly/3iMw7p5");
}

void displayHowTo() {
  handleJoystickPress();
  lcd.setCursor(0, 0);
  lcd.print("Move joystick");
  lcd.setCursor(0, 1);
  lcd.print("Eat the food");
}