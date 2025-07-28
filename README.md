# Nerf Laser Tag Scoring System

A complete laser tag scoring system for the Adafruit Circuit Playground Express that detects Nerf LaserOps IR signals and provides real-time visual score tracking.

![Circuit Playground Express](https://cdn-shop.adafruit.com/230x173/3333-03.jpg)

## Hardware Requirements

- **Adafruit Circuit Playground Express** 
- maybe a cable, battery, or 3D printed case for it.


## Compatible Nerf Blasters

This system detects IR signals from **Nerf LaserOps** series blasters:
- Nerf LaserOps Pro AlphaPoint
- Nerf LaserOps Pro DeltaBurst
- Other LaserOps compatible blasters

## Libraries Required

Install these libraries through the Arduino IDE Library Manager:

```
IRLib2 by Chris Young (you may need to download this directly from github)
├── IRLibDecodeBase
├── IRLib_HashRaw  
├── IRLibCombo
└── IRLibRecv

Adafruit NeoPixel by Adafruit
```

**Note**: This project uses IRLib2 for IR detection instead of the Circuit Playground library to avoid conflicts and ensure reliable Nerf LaserOps signal detection.

## Installation

1. Install the Arduino IDE
2. Add Adafruit SAMD board support via Board Manager
3. Install required libraries (listed above)
4. Select **"Adafruit Circuit Playground Express"** as your board
5. Upload the code to your Circuit Playground Express

## Game Flow & LED Patterns

### 1. Startup (Power On)
**LED Pattern**: Alternating red and blue LEDs flash 6 times around the circle
```
Red-Blue-Red-Blue-Red-Blue-Red-Blue-Red-Blue (flash)
```

### 2. Setup Mode (Select Hit Points)
**LED Pattern**: White LEDs show selected hit points (1-10 LEDs = 10-100 points)

**Controls**:
- **Right Button**: Increment hit points (10→20→30...→100→10)
- **Left Button**: Confirm selection and start game

**LED Display**:
- 1 white LED = 10 hit points total (5 each team)
- 2 white LEDs = 20 hit points  
- ...
- 10 white LEDs = 100 hit points

**Visual Feedback**: Two white flashes when starting game

### 3. Active Game Mode
**LED Pattern**: Proportional team score display

**Score Display Logic**:
- **Red LEDs**  Show red team score proportion
- **Blue LEDs**  Show blue team score proportion  
- **Purple LED**: Appears when total points aren't evenly divisible (e.g., 11 red, 9 blue = 5 red + 1 purple + 4 blue)


**Hit Detection**:
- **Red Team Hit**: All LEDs flash red, red score increases, blue score decreases
- **Blue Team Hit**: All LEDs flash blue, blue score increases, red score decreases

### 4. Game Over
**LED Pattern**: Winning team color flashes continuously

**Reset**: Press **Left Button** to return to Setup Mode with startup animation

## IR Signal Detection

The system detects the following Nerf LaserOps IR hash codes:

```cpp
RED Team:   0x78653B0E
BLUE Team:  0x2FFEA610  
PURPLE:     0x67228B44 (logged but no score effect)
```

Unknown IR patterns are logged to Serial Monitor for debugging.

## Serial Monitor Output

Connect to Serial Monitor (115200 baud) to see:
- Button press confirmations
- Hit point selection
- IR hit detection with hex codes
- Score updates
- Game state changes

Example output:
```
Nerf Laser Tag Scoring System Ready
*** RIGHT BUTTON PRESSED - INCREMENTING HITPOINTS ***
New hitpoints: 30
*** LEFT BUTTON PRESSED - STARTING GAME ***
Game started with 30 total hitpoints
RED HIT DETECTED! (Pattern: 0x78653B0E)
Score - Red: 16 Blue: 14
BLUE HIT DETECTED! (Pattern: 0x2FFEA610)  
Score - Red: 15 Blue: 15
```

## Technical Details

### Button Implementation
Uses Circuit Playground Express hardware-specific button handling:
```cpp
pinMode(CPLAY_LEFTBUTTON, INPUT_PULLDOWN);   // Pin 4
pinMode(CPLAY_RIGHTBUTTON, INPUT_PULLDOWN);  // Pin 5
```

### IR Reception
Optimized for maximum sensitivity:
- IR detection prioritized in main loop
- Minimal LED update interference
- Same timing patterns as proven utility code

### Scoring System
- **Zero-sum scoring**: One team's gain = other team's loss
- **Single variable tracking**: Only red score tracked, blue = total - red
- **Proportional display**: LEDs represent percentage of total points


## Troubleshooting

**Buttons not responding**: Ensure you have a Circuit Playground Express (not Classic)
**IR not detecting**: Check Serial Monitor for unknown pattern codes
**LEDs not displaying correctly**: Verify NeoPixel library installation
**Compilation errors**: Ensure IRLib2 and Adafruit NeoPixel libraries are installed

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for improvements:
- Additional Nerf blaster support
- New game modes
- UI enhancements  
- Bug fixes

## Hardware Links

- [Circuit Playground Express](https://www.adafruit.com/product/3333) - Adafruit
- [Nerf LaserOps Pro AlphaPoint](https://nerf.hasbro.com/) - Hasbro  
- [Nerf LaserOps Pro DeltaBurst](https://nerf.hasbro.com/) - Hasbro
