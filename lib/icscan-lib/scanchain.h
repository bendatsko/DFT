#ifndef SCANCHAIN_H
#define SCANCHAIN_H

#include <Arduino.h>

class ScanChain {
public:
    // Define the size of the internal buffer for data transfer
    static const size_t BUFFER_SIZE = 1024; // Buffer size in bytes
    static const int MAX_CHAIN_LENGTH = 32;

    // Constructor: Initializes scan chain with specified parameters
    // chainLength: Length of each shift register in the scan chain
    // clockPin: Pin number for the clock signal
    // dataIn1/2/3, dataOut1/2/3: Pin numbers for data input/output for each channel
    // enable1/2/3: Pin numbers for enable signals for each channel
    ScanChain(int chainLength, int clockPin,
              int dataIn1, int dataOut1, int enable1,
              int dataIn2, int dataOut2, int enable2,
              int dataIn3, int dataOut3, int enable3);

    // Control methods
    void begin(unsigned long clockSpeed); // Initialize scan chain with a specified clock speed
    void run();   // Start the scan chain operation
    void stop();  // Stop the scan chain operation
    void clear(); // Clear all data and reset the scan chain

    // Data loading methods
    void loadData(uint32_t data1, uint32_t data2, uint32_t data3); // Load 32 bits of data into each of the three channels
    void loadDataFromMemory(const uint8_t* data, size_t dataSize); // Load data from memory for transfer

    // Status and output methods
    bool isComplete() const; // Check if the data transfer is complete
    void getOutput(uint32_t& out1, uint32_t& out2, uint32_t& out3) const; // Get the current output of the scan chain

    // Callback functionality
    typedef void (*DataTransferCallback)(void);
    void setDataTransferCallback(DataTransferCallback callback); // Set the callback function for chunk transfers

    bool verifyData(const uint8_t* originalData, size_t dataSize);
    void setDebugMode(bool enable) {
        debugMode = enable;
    }

private:
    bool debugMode;

    // Static members
    static void timerISR();              	// Timer Interrupt Service Routine
    static ScanChain* instance;          	// Pointer to the current instance (used in ISR)

    // Configuration
    const int chainLength;               	// Length of each shift register in the scan chain
    const int clockPin;                  	// Pin number for the clock signal
    const int dataPins[3][2];            	// Pin numbers for data in/out for each channel [channel][in/out]
    const int enablePins[3];             	// Pin numbers for enable signals for each channel

    // State variables
    volatile uint32_t shiftRegisters[3]; 	// Shift registers for each channel
    volatile bool scanComplete;          	// Flag to indicate if a scan operation is complete
    volatile bool running;               	// Flag to indicate if the scan chain is currently running

    // Timing
    unsigned long clockPeriodUs;         	// Clock period in microseconds
    IntervalTimer timer;                 	// Timer object for generating clock signals

    // Data transfer
    volatile const uint8_t* dataPointer; 	// Pointer to the data being transferred
    volatile size_t remainingDataSize;   	// Remaining size of data to be transferred
    uint8_t volatile buffer[BUFFER_SIZE];	// Internal buffer for data transfer
    volatile size_t bufferIndex;         	// Current index in the buffer
    volatile bool bufferEmpty;           	// Flag to indicate if the buffer is empty
    DataTransferCallback transferCallback; 	// Callback function for chunk transfers

    // Helper method
    void loadNextChunk();                	// Load the next chunk of data into the buffer
};

#endif // SCANCHAIN_H