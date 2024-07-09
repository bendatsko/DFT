// =======================================================================================
// Flynn Research Group - University of Michigan
// =======================================================================================
//
// File Name: main.cpp
//
// Purpose:
// Demonstrates scan chain functionality by transferring 32KB of easily-debuggable data
// through three parallel channels.
//
// The ScanChain class manages a scan chain with three parallel channels, each with
// its own data input, data output, and enable pins. It can transfer large amounts of
// data efficiently using an internal buffer and interrupt-driven timing.
//
// Revision History:
// Date        | Author Name      | Comment
// ------------+------------------+--------------------------------------------------
// 24-Jul-8    | Benjamin Datsko  | Initial Version
// 			   | 			      |
// =======================================================================================

#include <Arduino.h>
#include "../lib/icscan-lib/scanchain.h" // <-- SCAN CHAIN LIBRARY 

// ---------------------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------------------

// Pin Definitions
const int CLOCK_PIN = 31;
const int DATA_IN_1 = 33,  DATA_OUT_1 = 34, ENABLE_1 = 16;  // Channel 1 pins
const int DATA_IN_2 = 35,  DATA_OUT_2 = 36, ENABLE_2 = 17;  // Channel 2 pins
const int DATA_IN_3 = 37,  DATA_OUT_3 = 38, ENABLE_3 = 0;   // Channel 3 pins

// Scan Chain Configuration
const int CHAIN_LENGTH = 8;  // Number of bits in each shift register
const unsigned long CLOCK_SPEED = 100000;  // 100 kHz clock speed

// Data Configuration
const size_t DATA_SIZE = 32 * 1024;  // 32 KB of test data

// ---------------------------------------------------------------------------------------
// Global Variables
// ---------------------------------------------------------------------------------------

// Initialize a ScanChain object
ScanChain scanChain(CHAIN_LENGTH,
                    CLOCK_PIN,
                    DATA_IN_1, DATA_OUT_1, ENABLE_1,
                    DATA_IN_2, DATA_OUT_2, ENABLE_2,
                    DATA_IN_3, DATA_OUT_3, ENABLE_3);

// Test data array
uint8_t largeData[DATA_SIZE];

// ---------------------------------------------------------------------------------------
// Setup Function
// ---------------------------------------------------------------------------------------

void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Wait for serial connection (with timeout)
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 5000));

    // Initialize test data
    // The data pattern is 1, 2, 3, 4, ..., which wraps around after 255
    // This allows easy debugging:
    // + Channel 1 should transfer 1, 4, 7, ...
    // + Channel 2 should transfer 2, 5, 8, ...
    // + Channel 3 should transfer 3, 6, 9, ...
    for (size_t i = 0; i < DATA_SIZE; i++) {
        largeData[i] = (i % 255) + 1;
    }

    // Debug mode (i.e., toggle verbose CLI output)
    scanChain.setDebugMode(true); // Enable debug output

    // Start up the scan chain at our clock speed
    scanChain.begin(CLOCK_SPEED);

    // Load test data into teensy memory
    scanChain.loadDataFromMemory(largeData, DATA_SIZE);

    // Run the scan chain
    scanChain.run();
    Serial.println("Scan chain started. Transferring data...");
}

// ---------------------------------------------------------------------------------------
// Main Loop
// ---------------------------------------------------------------------------------------

void loop() {
    static unsigned long startTime = millis();
    static const unsigned long TIMEOUT = 30000; // 30 seconds timeout

    if (scanChain.isComplete()) {
        scanChain.stop();
        Serial.println("Data transfer complete.");
        if (scanChain.verifyData(largeData, DATA_SIZE)) {
            Serial.println("Data verified successfully.");
        } else {
            Serial.println("Data verification failed!");
        }
        while (1); // Stop execution
    }

    if (millis() - startTime > TIMEOUT) {
        Serial.println("Operation timed out!");
        scanChain.stop();
        while (1); // Stop execution
    }

    // Progress indicator
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {  // Print every second
        Serial.println("Data transfer in progress...");
        lastPrint = millis();
    }
}