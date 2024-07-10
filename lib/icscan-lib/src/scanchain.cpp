#include "../scanchain.h"
#include <Arduino.h>

ParallelScanChain::ParallelScanChain(int clockPin,
                                     int dataIn1, int dataOut1,
                                     int dataIn2, int dataOut2,
                                     int dataIn3, int dataOut3,
                                     int chainLength)
    : _clockPin(clockPin), _chainLength(chainLength) {
    _dataInPins[0] = dataIn1;
    _dataInPins[1] = dataIn2;
    _dataInPins[2] = dataIn3;
    _dataOutPins[0] = dataOut1;
    _dataOutPins[1] = dataOut2;
    _dataOutPins[2] = dataOut3;

    if (_chainLength > MAX_CHAIN_LENGTH) {
        _chainLength = MAX_CHAIN_LENGTH;
    }
}

void ParallelScanChain::begin() {
    pinMode(_clockPin, OUTPUT);
    digitalWrite(_clockPin, LOW);

    for (int i = 0; i < 3; i++) {
        pinMode(_dataInPins[i], INPUT);
        pinMode(_dataOutPins[i], OUTPUT);
        digitalWrite(_dataOutPins[i], LOW);
    }

    reset();
}

void ParallelScanChain::scanIn(uint32_t data1, uint32_t data2, uint32_t data3) {
    uint32_t data[3] = {data1, data2, data3};

    Serial.println("Scanning in:");
    Serial.println(data1, BIN);
    Serial.println(data2, BIN);
    Serial.println(data3, BIN);

    for (int i = 0; i < _chainLength; i++) {
        _shiftRegisters[0][i] = (data1 >> (31 - i)) & 1;
        _shiftRegisters[1][i] = (data2 >> (31 - i)) & 1;
        _shiftRegisters[2][i] = (data3 >> (31 - i)) & 1;
    }

    Serial.println("Scan-in complete. New state:");
    for (int i = 0; i < _chainLength; i++) {
        Serial.print(_shiftRegisters[0][i]);
        Serial.print(_shiftRegisters[1][i]);
        Serial.print(_shiftRegisters[2][i]);
        Serial.print(" ");
    }
    Serial.println();
}

void ParallelScanChain::scanOut(uint32_t& data1, uint32_t& data2, uint32_t& data3) {
    uint32_t data[3] = {0, 0, 0};

    for (int bit = 0; bit < 32; bit++) {
        for (int bus = 0; bus < 3; bus++) {
            data[bus] = (data[bus] << 1) | digitalRead(_dataInPins[bus]);
        }
        shiftOnce();
    }

    data1 = data[0];
    data2 = data[1];
    data3 = data[2];
}

void ParallelScanChain::shiftOnce() {
    for (int bus = 0; bus < 3; bus++) {
        for (int i = _chainLength - 1; i > 0; i--) {
            _shiftRegisters[bus][i] = _shiftRegisters[bus][i-1];
        }
        _shiftRegisters[bus][0] = digitalRead(_dataInPins[bus]);
    }
    clockPulse();

    Serial.println("Shifted once. New state:");
    for (int i = 0; i < _chainLength; i++) {
        Serial.print(_shiftRegisters[0][i]);
        Serial.print(_shiftRegisters[1][i]);
        Serial.print(_shiftRegisters[2][i]);
        Serial.print(" ");
    }
    Serial.println();
}

void ParallelScanChain::reset() {
    for (int bus = 0; bus < 3; bus++) {
        for (int i = 0; i < _chainLength; i++) {
            _shiftRegisters[bus][i] = 0;
        }
    }
}

void ParallelScanChain::clockPulse() {
    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(1);  // Adjust this delay as needed
    digitalWrite(_clockPin, LOW);
    delayMicroseconds(1);  // Adjust this delay as needed
}

uint32_t ParallelScanChain::getRegisterValue(int busIndex, int position) const {
    if (busIndex >= 0 && busIndex < 3 && position >= 0 && position < _chainLength) {
        return _shiftRegisters[busIndex][position];
    }
    return 0;
}