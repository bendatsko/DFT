#ifndef PARALLEL_SCAN_CHAIN_H
#define PARALLEL_SCAN_CHAIN_H

#include <Arduino.h>

class ParallelScanChain {
public:
    ParallelScanChain(int clockPin,
                      int dataIn1, int dataOut1,
                      int dataIn2, int dataOut2,
                      int dataIn3, int dataOut3,
                      int chainLength);
    void begin();
    void scanIn(uint32_t data1, uint32_t data2, uint32_t data3);
    void scanOut(uint32_t& data1, uint32_t& data2, uint32_t& data3);
    void shiftOnce();
    void reset();
    uint32_t getRegisterValue(int busIndex, int position) const;

private:
    static const int MAX_CHAIN_LENGTH = 32;  // Maximum chain length (32 bits)

    int _clockPin;
    int _dataInPins[3];
    int _dataOutPins[3];
    int _chainLength;

    uint32_t _shiftRegisters[3][MAX_CHAIN_LENGTH];

    void clockPulse();
};

#endif // PARALLEL_SCAN_CHAIN_H