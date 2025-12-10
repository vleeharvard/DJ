#ifndef TOUCH_WHEEL_H
#define TOUCH_WHEEL_H

#include <Arduino.h>
#include "Touch_Sensor.h"
#include "Utils.h"

class Touch_Wheel {

    private:
    Touch_Sensor** sensors;
    
    bool last_button_state = 0;
    unsigned long last_button_pressed_time = 0;

    int last_wheel_state = 0;
    unsigned long last_wheel_pressed_time = 0;

    int swipe_state = 0;
    unsigned long last_swipe_pressed_time = 0;


    public:
    explicit Touch_Wheel(Touch_Sensor** sensors);
    
    int16_t wheel();
    char swipe();
    bool button();
};

#endif // WAV_FILE_H