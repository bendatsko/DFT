# DFTFriend: Multi-Channel ASIC Testing Framework

## Project Overview
DFTFriend is a multi-channel parallel scan chain framework designed for ASIC testing and validation. It implements Design for Testability (DFT) principles to provide an efficient solution for interfacing with exposed scan pins on ASICs.

## Key Features
- Parallel operation of multiple scan chain buses
- Configurable clock speeds (tested up to 20 MHz)
- Support for large data transfers (32 KB tested)
- Flexible pin configuration
- Interrupt-driven timing control
- Real-time data verification

## Technical Specifications
- Target Platform: Teensy 4.1 (ARM Cortex-M7)
- Language: C++ (Arduino framework)
- Buffer Size: 1024 bytes (configurable)
- Channels: 3 independent scan chains

## Dependencies
- Arduino IDE (1.8.x or later) or PlatformIO (5.x or later)
- Teensyduino Add-on (if using Arduino IDE)
- Teensy 4.1 Board Support

## Installation
1. Clone the repository:
   ```
   git clone https://github.com/bendatsko/DFTFriend.git
   ```
2. For Arduino IDE: Copy `src/scanchain.cpp` and `include/scanchain.h` to your Arduino libraries folder.
3. For PlatformIO: Open the cloned repository in your PlatformIO-supported IDE.

## Basic Usage
```cpp
#include <scanchain.h>

ScanChain scanChain(CHAIN_LENGTH, CLOCK_PIN, 
                    DATA_IN_1, DATA_OUT_1, ENABLE_1,
                    DATA_IN_2, DATA_OUT_2, ENABLE_2, 
                    DATA_IN_3, DATA_OUT_3, ENABLE_3);

void setup() {
    scanChain.begin(CLOCK_SPEED);
    scanChain.loadDataFromMemory(testData, DATA_SIZE);
    scanChain.run();
}

void loop() {
    if (scanChain.isComplete()) {
        if (scanChain.verifyData(testData, DATA_SIZE)) {
            Serial.println("Data transfer successful and verified.");
        }
    }
}
```

For more detailed examples, refer to the `examples` folder in the repository.