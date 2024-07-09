#include "../scanchain.h"

// Initialize static member
ScanChain* ScanChain::instance = nullptr;

ScanChain::ScanChain(int chainLength, int clockPin,
                     int dataIn1, int dataOut1, int enable1,
                     int dataIn2, int dataOut2, int enable2,
                     int dataIn3, int dataOut3, int enable3)
    : chainLength(chainLength > MAX_CHAIN_LENGTH ? MAX_CHAIN_LENGTH : chainLength),
      clockPin(clockPin),
      dataPins{{dataIn1, dataOut1}, {dataIn2, dataOut2}, {dataIn3, dataOut3}},
      enablePins{enable1, enable2, enable3},
      scanComplete(false),
      running(false),
      dataPointer(nullptr),
      remainingDataSize(0),
      bufferIndex(0),
      bufferEmpty(true),
      transferCallback(nullptr),
      debugMode(false)
{
    instance = this;
}

// Create scan chain with a clock speed
void ScanChain::begin(unsigned long clockSpeed) {
    // Configure pins
    for (int i = 0; i < 3; i++) {
        pinMode(dataPins[i][0], INPUT);  // Set data input pins
        pinMode(dataPins[i][1], OUTPUT); // Set data output pins
        pinMode(enablePins[i], OUTPUT);  // Set enable pins to output
        digitalWrite(enablePins[i], LOW);  // Set enable pins to LOW (active low, so this enables)
    }

    // Calculate clock period in microseconds
    clockPeriodUs = 1000000 / clockSpeed;

    // Set up timer interrupt service routine (ISR)
    timer.begin(timerISR, clockPeriodUs);
    timer.priority(255); // Highest priority
}

// Start the scan chain operation
void ScanChain::run() {
    noInterrupts();         // Temporarily disable interrupts
    running = true;         // Set running flag
    scanComplete = false;   // Reset scan complete flag
    interrupts();           // Reenable interrupts and proceed
}

// Stop the scan chain operation
void ScanChain::stop() {
    noInterrupts();         // Temporarily disable interrupts
    running = false;        // Clear running flag
    interrupts();           // Reenable interrupts and proceed
}

// Clear all data and reset the scan chain
void ScanChain::clear() {
    noInterrupts();
    for (int i = 0; i < 3; i++) {
        shiftRegisters[i] = 0;  // Reset all shift registers to 0
    }
    scanComplete = false;       // Reset states
    dataPointer = nullptr;
    remainingDataSize = 0;
    bufferIndex = 0;
    bufferEmpty = true;
    interrupts();
}

// Place next 32 bits of data into each of the three channels
void ScanChain::loadData(uint32_t data1, uint32_t data2, uint32_t data3) {
    noInterrupts();
    shiftRegisters[0] = data1;  // Load data for channel 1
    shiftRegisters[1] = data2;  // Load data for channel 2
    shiftRegisters[2] = data3;  // Load data for channel 3
    scanComplete = false;       // Reset scan complete flag
    interrupts();
}

// Load data from memory for transfer
void ScanChain::loadDataFromMemory(const uint8_t* data, size_t dataSize) {
    noInterrupts();
    dataPointer = data; // Set pointer to the start of the data
    remainingDataSize = dataSize;
    bufferIndex = 0;
    bufferEmpty = true;
    loadNextChunk();    // Load the first chunk of data
    interrupts();
}

// Load the next chunk of data into the buffer
void ScanChain::loadNextChunk() {
    size_t bytesToCopy = min(remainingDataSize, BUFFER_SIZE);
    memcpy(const_cast<void*>(static_cast<volatile void*>(buffer)),
           const_cast<const void*>(static_cast<const volatile void*>(dataPointer)),
           bytesToCopy);
    dataPointer += bytesToCopy;
    remainingDataSize -= bytesToCopy;
    bufferIndex = 0;
    bufferEmpty = false;
}

// Check if the data transfer is complete
bool ScanChain::isComplete() const {
    return  scanComplete               // Current scan cycle is complete
            && remainingDataSize == 0  // No more data to transfer
            && bufferEmpty;            // Buffer is empty
}

// Get the current output of the scan chain
void ScanChain::getOutput(uint32_t& out1, uint32_t& out2, uint32_t& out3) const {
    noInterrupts();
    out1 = shiftRegisters[0];  // Get output for channel 1
    out2 = shiftRegisters[1];  // Get output for channel 2
    out3 = shiftRegisters[2];  // Get output for channel 3
    interrupts();              // Reenable interrupts
}

// Set the callback function for chunk transfers
void ScanChain::setDataTransferCallback(DataTransferCallback callback) {
    transferCallback = callback; // Set the callback function
}

bool ScanChain::verifyData(const uint8_t* originalData, size_t dataSize) {
    if (dataSize != remainingDataSize + bufferIndex) {
        return false; // Data size mismatch
    }

    // Check data in buffer
    for (size_t i = 0; i < bufferIndex; i++) {
        if (buffer[i] != originalData[i]) {
            return false;
        }
    }

    // Check remaining data
    for (size_t i = 0; i < remainingDataSize; i++) {
        if (dataPointer[i] != originalData[bufferIndex + i]) {
            return false;
        }
    }

    return true;
}

void ScanChain::timerISR() {
    static bool clockState = false; // Tracks the current state of the clock (high/low)
    static int bitCount = 0;        // Counts bits shifted in current scan cycle

    // Respond to clock
    if (instance && instance->running) {
        if (clockState) {
            // Clock high: Shift in data
            digitalWriteFast(instance->clockPin, HIGH);
            delayMicroseconds(1);  // Small delay for signal stability

            for (int i = 0; i < 3; i++) {
                // Shift in new bit from data input pin for each channel
                instance->shiftRegisters[i] =
                    (instance->shiftRegisters[i] << 1) |
                    digitalReadFast(instance->dataPins[i][0]); // Shift left and add new bit
            }
            bitCount++; // Increment bit count for this cycle
        }
        else {
            // Clock low: Output data
            digitalWriteFast(instance->clockPin, LOW);
            delayMicroseconds(1);  // Small delay for signal stability

            for (int i = 0; i < 3; i++) {
                // Output MOST SIGNIFICANT BIT to data output pin for each channel
                digitalWriteFast(
                    instance->dataPins[i][1],
                    (instance->shiftRegisters[i] >> (instance->chainLength - 1)) & 1);
            }
        }

        clockState = !clockState; // Toggle clock state

        // Check if we've shifted all bits in the chain
        if (bitCount == instance->chainLength) {
            bitCount = 0;
            instance->scanComplete = true;

            // Load next data from buffer if available
            if (!instance->bufferEmpty) {
                for (int i = 0; i < 3; i++) {
                    if (instance->bufferIndex < instance->BUFFER_SIZE) {
                        instance->shiftRegisters[i] =
                            instance->buffer[instance->bufferIndex++];
                    }
                }

                // Check if we've used all data in the buffer
                if (instance->bufferIndex >= instance->BUFFER_SIZE) {
                    instance->bufferEmpty = true;

                    // Load next chunk if there's more data
                    if (instance->remainingDataSize > 0) {
                        instance->loadNextChunk();
                    }

                    // Call the transfer callback if set
                    if (instance->transferCallback) {
                        instance->transferCallback();
                    }
                }
            }
        }
        if (instance->debugMode) {
            Serial.print("Channel outputs: ");
            for (int i = 0; i < 3; i++) {
                Serial.print(instance->shiftRegisters[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
}