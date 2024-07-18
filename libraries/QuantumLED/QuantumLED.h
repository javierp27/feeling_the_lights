#ifndef QUANTUMLED_H
#define QUANTUMLED_H

#include <Adafruit_NeoPixel.h>

template<size_t N, size_t N_LED>
class Quantum {
public:
    Quantum(uint32_t color, int velocity, const int (&tailIntensities)[N], int direction, int spin);

    void move();
    void display(uint32_t* ledColors, uint32_t* headPositions, int volume);
    void reverseDirection();
    int getNextPosition() const;
    int getPosition() const;
    int getDirection() const;
    int getSpin() const;
    void changeVelocity(int new_velocity);
    void changeSpin(int new_spin);
    void setShift(int new_shift);

private:
    uint32_t color;
    int velocity;
    int position;
    int direction;
    int counter;
    const int (&tailIntensities)[N];
    int tailLength;
    int spin;
    int shift;

    uint32_t blendColors(uint32_t color1, uint32_t color2) const;
    uint32_t adjustColorIntensity(uint32_t color, int intensityPercent) const;
};

void handleCollisions(Quantum<4, 16>& q1, Quantum<4, 16>& q2);

template<size_t N1, size_t N2, size_t N_LED>
void handleCollisions(Quantum<N1, N_LED>& q1, Quantum<N2, N_LED>& q2);

int getVolume(int micPin);

#include "QuantumLED.tpp"

#endif
