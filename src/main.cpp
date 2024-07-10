#include <Arduino.h>
#include "../lib/icscan-lib/scanchain.h"
#include <Adafruit_NeoPixel.h>

#define CLOCK_PIN 31
#define DATA_IN_1 33
#define DATA_OUT_1 34
#define DATA_IN_2 35
#define DATA_OUT_2 36
#define DATA_IN_3 37
#define DATA_OUT_3 38
#define CHAIN_LENGTH 10

#define LED_PIN 34
#define NUM_LEDS 10  // We'll use one LED per scan chain position

ParallelScanChain scanChain(CLOCK_PIN, DATA_IN_1, DATA_OUT_1, DATA_IN_2, DATA_OUT_2, DATA_IN_3, DATA_OUT_3, CHAIN_LENGTH);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void updateLEDs() {
    Serial.println("Updating LEDs:");
    for (int i = 0; i < CHAIN_LENGTH; i++) {
        uint32_t color = 0;
        bool red = scanChain.getRegisterValue(0, i);
        bool green = scanChain.getRegisterValue(1, i);
        bool blue = scanChain.getRegisterValue(2, i);

        if (red) color |= 0xFF0000;
        if (green) color |= 0x00FF00;
        if (blue) color |= 0x0000FF;

        strip.setPixelColor(i, color);

        Serial.print("LED ");
        Serial.print(i);
        Serial.print(": R=");
        Serial.print(red);
        Serial.print(" G=");
        Serial.print(green);
        Serial.print(" B=");
        Serial.print(blue);
        Serial.print(" Color=0x");
        Serial.println(color, HEX);
    }
    strip.show();
}

void setup() {
    Serial.begin(9600);
    while (!Serial);

    scanChain.begin();
    strip.begin();
    strip.setBrightness(50); // Set to a lower brightness
    strip.show();  // Initialize all pixels to 'off'

    Serial.println("Parallel ScanChain Test");
    Serial.println("1. Scan In Test Pattern");
    Serial.println("2. Shift Once");
    Serial.println("3. Reset");
    Serial.println("Enter test number to run:");
}

void loop() {
    if (Serial.available()) {
        int testCase = Serial.parseInt();
        Serial.println("Running test case " + String(testCase));

        switch (testCase) {
        case 1: {
                // Scan in a simple pattern:
                // Bus 1 (Red):   1010101010
                // Bus 2 (Green): 0101010101
                // Bus 3 (Blue):  1100110011
                uint32_t data1 = 0b10101010100000000000000000000000;
                uint32_t data2 = 0b01010101010000000000000000000000;
                uint32_t data3 = 0b11001100110000000000000000000000;
                scanChain.scanIn(data1, data2, data3);
                updateLEDs();
                Serial.println("Test pattern scanned in.");
                break;
        }
        case 2:
            scanChain.shiftOnce();
            updateLEDs();
            Serial.println("Shifted once.");
            break;
        case 3:
            scanChain.reset();
            updateLEDs();
            Serial.println("Reset complete.");
            break;
        default:
            Serial.println("Invalid test case");
            return;
        }

        // Print the state of the scan chain
        Serial.println("Scan Chain State:");
        for (int i = 0; i < CHAIN_LENGTH; i++) {
            Serial.print(scanChain.getRegisterValue(0, i));
            Serial.print(scanChain.getRegisterValue(1, i));
            Serial.print(scanChain.getRegisterValue(2, i));
            Serial.print(" ");
        }
        Serial.println();
    }
}