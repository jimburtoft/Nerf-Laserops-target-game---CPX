/* Nerf LaserTag Scoring System for Circuit Playground Express
 *
 * Features:
 * - Startup: Flash red/blue alternating LEDs
 * - Setup: Select hitpoints (10-100) using A/B buttons
 * - Game: Display proportional red/blue scores, process hits
 * - End: Flash winning color until restart
 */

#define CPLAY_NEOPIXELPIN 8 // neopixel pin
#define CPLAY_IR_RECEIVER 26 // IR receiver pin
#define CPLAY_LEFTBUTTON 4     // Left button pin (using library method)
#define CPLAY_RIGHTBUTTON 5    // Right button pin (using library method)

// Use only IRLib2 (your working IR setup)
#include <IRLibDecodeBase.h>
#include <IRLib_HashRaw.h>
#include <IRLibCombo.h>
IRdecode myDecoder;
#include <IRLibRecv.h>
IRrecv myReceiver(CPLAY_IR_RECEIVER);

// NeoPixel library
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, CPLAY_NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// Timing constants
static const unsigned long ANIMATION_DELAY = 50; //ms
static const unsigned long HOLD_DELAY = 250; //ms
static const unsigned long FLASH_DELAY = 200; //ms
static const unsigned long DEBOUNCE_DELAY = 200; //ms

// IR hashes for Nerf LaserOps (from your working code)
// Multiple patterns for different Nerf systems
static const long PURPLE = 0x67228B44;
static const long RED1 = 0x78653B0E;    // First red pattern
static const long BLUE1 = 0x2FFEA610;   // First blue pattern
// Add more patterns as needed
static const long RED2 = 0x00000000;    // Second red pattern (update with actual value)
static const long BLUE2 = 0x00000000;   // Second blue pattern (update with actual value)

// Game states
enum GameState {
  STARTUP,
  SETUP_HITPOINTS,
  PLAYING,
  GAME_OVER
};

// Global variables
GameState currentState = STARTUP;
int totalHitpoints = 10;
int redScore = 5;  // Only track red score, blue = totalHitpoints - redScore
int selectedHitpoints = 1; // 1-10, representing 10-100 hitpoints
unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize button pins using Circuit Playground Express method
  #if defined(__SAMD21G18A__) || defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS)
    pinMode(CPLAY_LEFTBUTTON, INPUT_PULLDOWN);
    pinMode(CPLAY_RIGHTBUTTON, INPUT_PULLDOWN);
  #else
    pinMode(CPLAY_LEFTBUTTON, INPUT);
    pinMode(CPLAY_RIGHTBUTTON, INPUT);
  #endif
  
  // Initialize IR receiver (your working setup)
  myReceiver.enableIRIn();
  
  // Initialize NeoPixels
  strip.begin();
  strip.setBrightness(64);
  strip.clear();
  strip.show();
  
  Serial.println(F("Nerf Laser Tag Scoring System Ready"));
  Serial.println(F("Buttons: Left=Confirm/A, Right=Increment/B"));
  Serial.println(F("IR Detection: Active for Nerf LaserOps"));
  
  // Start with startup animation
  startupAnimation();
  currentState = SETUP_HITPOINTS;
  
  Serial.println("Entering setup mode - use buttons to select hitpoints");
}

void loop() {
  switch(currentState) {
    case SETUP_HITPOINTS:
      handleHitpointSetup();
      break;
    case PLAYING:
      handleGameplay();
      break;
    case GAME_OVER:
      handleGameOver();
      break;
  }
  delay(10);
}

void startupAnimation() {
  for(int flash = 0; flash < 6; flash++) {
    strip.clear();
    for(int i = 0; i < 10; i++) {
      if(i % 2 == 0) {
        strip.setPixelColor(i, 255, 0, 0); // Red
      } else {
        strip.setPixelColor(i, 0, 0, 255); // Blue
      }
    }
    strip.show();
    delay(FLASH_DELAY);
    
    strip.clear();
    strip.show();
    delay(FLASH_DELAY);
  }
}

void handleHitpointSetup() {
  // Display current hitpoint selection as white LEDs
  strip.clear();
  for(int i = 0; i < selectedHitpoints; i++) {
    strip.setPixelColor(i, 255, 255, 255); // White
  }
  strip.show();
  
  // Check buttons using Circuit Playground Express method
  bool leftButtonPressed = digitalRead(CPLAY_LEFTBUTTON);
  bool rightButtonPressed = digitalRead(CPLAY_RIGHTBUTTON);
  
  static unsigned long lastDebug = 0;
  static bool lastLeft = false;
  static bool lastRight = false;
  
  if(millis() - lastDebug > 500) {
    Serial.print("Left: ");
    Serial.print(leftButtonPressed ? 1 : 0);
    Serial.print(" Right: ");
    Serial.print(rightButtonPressed ? 1 : 0);
    Serial.print(" Hitpoints: ");
    Serial.println(selectedHitpoints * 10);
    lastDebug = millis();
  }
  
  // Left button pressed - start game
  if(leftButtonPressed && !lastLeft && millis() - lastButtonPress > DEBOUNCE_DELAY) {
    lastButtonPress = millis();
    Serial.println("*** LEFT BUTTON PRESSED - STARTING GAME ***");
    flashTwice();
    
    totalHitpoints = selectedHitpoints * 10;
    redScore = totalHitpoints / 2;
    currentState = PLAYING;
    
    Serial.print("Game started with ");
    Serial.print(totalHitpoints);
    Serial.println(" total hitpoints");
    Serial.print("Red: ");
    Serial.print(redScore);
    Serial.print(" Blue: ");
    Serial.println(totalHitpoints - redScore);
    
    updateScoreDisplay();
    lastLeft = leftButtonPressed;
    return;
  }
  
  // Right button pressed - increment hitpoints
  if(rightButtonPressed && !lastRight && millis() - lastButtonPress > DEBOUNCE_DELAY) {
    lastButtonPress = millis();
    Serial.println("*** RIGHT BUTTON PRESSED - INCREMENTING HITPOINTS ***");
    selectedHitpoints++;
    if(selectedHitpoints > 10) {
      selectedHitpoints = 1;
    }
    Serial.print("New hitpoints: ");
    Serial.println(selectedHitpoints * 10);
  }
  
  lastLeft = leftButtonPressed;
  lastRight = rightButtonPressed;
}

void handleGameplay() {
  // Check for IR hits FIRST (like your original utility)
  if(myReceiver.getResults()) {
    myDecoder.decode();
    if(myDecoder.protocolNum == UNKNOWN) {
      // Check for red hits (multiple patterns)
      if(myDecoder.value == RED1 || myDecoder.value == RED2) {
        Serial.print("RED HIT DETECTED! (Pattern: 0x");
        Serial.print(myDecoder.value, HEX);
        Serial.println(")");
        processHit(true); // Red hit
      } 
      // Check for blue hits (multiple patterns)
      else if(myDecoder.value == BLUE1 || myDecoder.value == BLUE2) {
        Serial.print("BLUE HIT DETECTED! (Pattern: 0x");
        Serial.print(myDecoder.value, HEX);
        Serial.println(")");
        processHit(false); // Blue hit
      } 
      // Check for purple hits
      else if(myDecoder.value == PURPLE) {
        Serial.print("PURPLE HIT DETECTED! (Pattern: 0x");
        Serial.print(myDecoder.value, HEX);
        Serial.println(")");
        // You can decide what purple does
      }
      // Unknown pattern - log it for debugging
      else {
        Serial.print("UNKNOWN IR PATTERN: 0x");
        Serial.println(myDecoder.value, HEX);
      }
    }
    // Use same delay as your original utility
    delay(HOLD_DELAY);
    // Clear display and re-enable IR (like your utility)
    strip.clear();
    strip.show();
    myReceiver.enableIRIn();
  }
  
  // Update score display AFTER IR check (less frequently)
  static unsigned long lastDisplayUpdate = 0;
  if(millis() - lastDisplayUpdate > 100) { // Update display every 100ms instead of every loop
    updateScoreDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Check for game over
  if(redScore >= totalHitpoints || redScore <= 0) {
    currentState = GAME_OVER;
    Serial.print("GAME OVER! Winner: ");
    Serial.println(redScore >= totalHitpoints ? "RED" : "BLUE");
  }
}

void handleGameOver() {
  bool redWins = (redScore >= totalHitpoints);
  
  // Flash winning color
  strip.clear();
  for(int i = 0; i < 10; i++) {
    if(redWins) {
      strip.setPixelColor(i, 255, 0, 0); // Red wins
    } else {
      strip.setPixelColor(i, 0, 0, 255); // Blue wins
    }
  }
  strip.show();
  delay(FLASH_DELAY);
  
  strip.clear();
  strip.show();
  delay(FLASH_DELAY);
  
  // Check for restart with left button
  bool leftButtonPressed = digitalRead(CPLAY_LEFTBUTTON);
  static bool lastLeft = false;
  
  if(leftButtonPressed && !lastLeft && millis() - lastButtonPress > DEBOUNCE_DELAY) {
    lastButtonPress = millis();
    Serial.println("*** LEFT BUTTON PRESSED - RESTARTING ENTIRE GAME ***");
    Serial.println("Returning to hitpoint selection...");
    
    // Reset everything back to the beginning
    selectedHitpoints = 1;
    totalHitpoints = 10;
    redScore = 5;
    currentState = SETUP_HITPOINTS;
    
    // Show startup animation again
    startupAnimation();
    
    Serial.println("Game completely reset!");
    Serial.println("Entering setup mode - use buttons to select hitpoints");
  }
  
  lastLeft = leftButtonPressed;
}

void flashTwice() {
  for(int i = 0; i < 2; i++) {
    strip.clear();
    for(int j = 0; j < 10; j++) {
      strip.setPixelColor(j, 255, 255, 255); // White flash
    }
    strip.show();
    delay(FLASH_DELAY);
    
    strip.clear();
    strip.show();
    delay(FLASH_DELAY);
  }
}

void processHit(bool isRedHit) {
  if(isRedHit) {
    Serial.println("RED HIT!");
    redScore++;
    
    // Flash all LEDs red (like your utility)
    for(int i = 0; i < 10; i++) {
      strip.setPixelColor(i, 255, 0, 0);
      strip.show();
      delay(ANIMATION_DELAY);
    }
  } else {
    Serial.println("BLUE HIT!");
    redScore--;
    
    // Flash all LEDs blue (like your utility)
    for(int i = 0; i < 10; i++) {
      strip.setPixelColor(i, 0, 0, 255);
      strip.show();
      delay(ANIMATION_DELAY);
    }
  }
  
  // Same final delay as your utility
  delay(HOLD_DELAY);
  
  int blueScore = totalHitpoints - redScore;
  Serial.print("Score - Red: ");
  Serial.print(redScore);
  Serial.print(" Blue: ");
  Serial.println(blueScore);
}

void updateScoreDisplay() {
  strip.clear();
  
  int blueScore = totalHitpoints - redScore;
  
  // Calculate LEDs for each team based on their score
  int redLEDs = (redScore * 10) / totalHitpoints;
  int blueLEDs = (blueScore * 10) / totalHitpoints;
  
  // Check if we need a purple LED for the remainder
  int totalUsedLEDs = redLEDs + blueLEDs;
  bool needPurple = (totalUsedLEDs < 10);
  
  // Light up red LEDs starting from position 0
  for(int i = 0; i < redLEDs; i++) {
    strip.setPixelColor(i, 255, 0, 0);
  }
  
  // Add purple LED if needed (represents the "contested" point)
  if(needPurple) {
    strip.setPixelColor(redLEDs, 128, 0, 128); // Purple
  }
  
  // Light up blue LEDs from the end, working backwards
  int blueStartPos = 10 - blueLEDs;
  for(int i = 0; i < blueLEDs; i++) {
    strip.setPixelColor(blueStartPos + i, 0, 0, 255);
  }
  
  strip.show();
  
  Serial.print("Score - Red: ");
  Serial.print(redScore);
  Serial.print("/");
  Serial.print(totalHitpoints);
  Serial.print(" (");
  Serial.print(redLEDs);
  Serial.print(" LEDs), Blue: ");
  Serial.print(blueScore);
  Serial.print("/");
  Serial.print(totalHitpoints);
  Serial.print(" (");
  Serial.print(blueLEDs);
  Serial.print(" LEDs)");
  if(needPurple) {
    Serial.print(", Purple: 1 LED");
  }
  Serial.println();
}
