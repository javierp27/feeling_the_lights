#include "QuantumLED.h"
#include <Arduino.h>

template<size_t N, size_t N_LED>
Quantum<N, N_LED>::Quantum(uint32_t color, int velocity, const int (&tailIntensities)[N], int direction, int spin)
  : color(color), velocity(velocity), position(0), direction(direction), counter(0), tailIntensities(tailIntensities), tailLength(N), spin(spin), shift(0) {}

template<size_t N, size_t N_LED>
void Quantum<N, N_LED>::move() {
    counter++;
    if (counter >= 30 / velocity) {
        position = (position + direction + N_LED) % N_LED;
        counter = 0;
    }
    int shifted_position = (position + shift + N_LED) % N_LED;
}

template<size_t N, size_t N_LED>
void Quantum<N, N_LED>::display(uint32_t* ledColors, uint32_t* headPositions, int volume) {

    int shifted_position = (position + shift*direction + N_LED) % N_LED;

    uint32_t adjustedColor = adjustColorIntensity(color, volume);
    uint32_t currentColor = ledColors[shifted_position];
    uint32_t newColor = blendColors(currentColor, adjustedColor);
    ledColors[position] = newColor;
    headPositions[position] = color;

    for (int i = 1; i <= tailLength; i++) {
        int tailPos = (shifted_position - i * direction + N_LED) % N_LED;
        uint32_t tailColor = adjustColorIntensity(adjustedColor, tailIntensities[i-1]);
        uint32_t currentTailColor = ledColors[tailPos];
        uint32_t newTailColor = blendColors(currentTailColor, tailColor);
        ledColors[tailPos] = newTailColor;
    }
}

template<size_t N, size_t N_LED>
void Quantum<N, N_LED>::reverseDirection() {
    direction = -direction;
}


template<size_t N, size_t N_LED>
int Quantum<N, N_LED>::getNextPosition() const {
    return (position + direction + N_LED) % N_LED;
}

template<size_t N, size_t N_LED>
int Quantum<N, N_LED>::getPosition() const {
    return position;
}

template<size_t N, size_t N_LED>
int Quantum<N, N_LED>::getDirection() const {
    return direction;
}

template<size_t N, size_t N_LED>
int Quantum<N, N_LED>::getSpin() const {
    return spin;
}

template<size_t N, size_t N_LED>
void Quantum<N, N_LED>::changeVelocity(int new_velocity) {
    velocity = new_velocity;
}

template<size_t N, size_t N_LED>
void Quantum<N, N_LED>::changeSpin(int new_spin) {
    spin = new_spin;
}

template<size_t N, size_t N_LED>
void Quantum<N, N_LED>::setShift(int new_shift) {  // Define the setShift function
    shift = new_shift;
}


template<size_t N, size_t N_LED>
uint32_t Quantum<N, N_LED>::blendColors(uint32_t color1, uint32_t color2) const {
    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >>  8) & 0xFF;
    uint8_t b1 =  color1        & 0xFF;

    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >>  8) & 0xFF;
    uint8_t b2 =  color2        & 0xFF;

    uint8_t r = min(r1 + r2, 255);
    uint8_t g = min(g1 + g2, 255);
    uint8_t b = min(b1 + b2, 255);

    return (r << 16) | (g << 8) | b;
}

template<size_t N, size_t N_LED>
uint32_t Quantum<N, N_LED>::adjustColorIntensity(uint32_t color, int intensityPercent) const {
    uint8_t r = ((color >> 16) & 0xFF) * intensityPercent / 100;
    uint8_t g = ((color >>  8) & 0xFF) * intensityPercent / 100;
    uint8_t b = (color        & 0xFF) * intensityPercent / 100;
    return (r << 16) | (g << 8) | b;
}

void handleCollisions(Quantum<4, 16>& q1, Quantum<4, 16>& q2) {
    int nextPosition1 = q1.getNextPosition();
    int nextPosition2 = q2.getNextPosition();

    if ((q1.getSpin() == q2.getSpin()) &&
        ((q1.getPosition() == nextPosition2 && q1.getDirection() != q2.getDirection()) ||
         (q2.getPosition() == nextPosition1 && q2.getDirection() != q1.getDirection()) ||
         (q1.getPosition() == q2.getPosition()))) {
        q1.reverseDirection();
        q2.reverseDirection();
    }
}

template<size_t N1, size_t N2, size_t N_LED>
void handleCollisions(Quantum<N1, N_LED>& q1, Quantum<N2, N_LED>& q2) {
    int nextPosition1 = q1.getNextPosition();
    int nextPosition2 = q2.getNextPosition();

    if ((q1.getSpin() == q2.getSpin()) &&
        ((q1.getPosition() == nextPosition2 && q1.getDirection() != q2.getDirection()) ||
         (q2.getPosition() == nextPosition1 && q2.getDirection() != q1.getDirection()) ||
         (q1.getPosition() == q2.getPosition()))) {
        q1.reverseDirection();
        q2.reverseDirection();
    }
}

int getVolume(int micPin) {
    int sum = 0;
    for (int i = 0; i < 400; i++) {
        int val = analogRead(micPin);
        sum += abs(val - 370);
        if (i % 45 == 0) {
            delay(1); // Añade un delay de 1 milisegundo cada 10 iteraciones
        }
    }
    return constrain(map(sum / 300, 0, 512, 0, 255), 0, 255);
}
