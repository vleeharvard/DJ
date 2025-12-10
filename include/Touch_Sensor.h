#include <Arduino.h>

#ifndef TOUCH_SENSOR_FILE_H
#define TOUCH_SENSOR_FILE_H

class Touch_Sensor {
    private:
    uint8_t pin;
    uint32_t lower;
    uint32_t upper;

    bool state = false; // Instantaneous state

    public:
    explicit Touch_Sensor(int pin, uint32_t lower, uint32_t upper);
    void init();
    uint32_t update();
    bool pressed();
    bool released();
};

#endif // TOUCH_SENSOR_H
