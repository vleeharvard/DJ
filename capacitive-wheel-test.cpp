#include <Arduino.h>
#include <Utils.h>
#include <SPI.h>
#include <driver/i2s.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- Custom libraries ---
#include "Wav_File.h"
#include "Channel.h"
#include "Utils.h"
#include "Touch_Sensor.h"

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "Touch_Sensor.h"
#include "Touch_Wheel.h"

Touch_Sensor* touch_sensors[NUM_SENSORS];
Touch_Wheel* touch_wheel;
int x = 0;

int counter = 0;
String swipe = "";

void setup() {
    Serial.begin(57600);
    Serial.printf("\n\n\nStarting Capacitive Wheel...\n");

    for (int i = 1; i <= 13; i++) {
        if (i == 1) { // Button [1]
            touch_sensors[i-1] = new Touch_Sensor(i, 30000, 60000);
        } else if (i <= 9) { // Wheel [2, 9]
            touch_sensors[i-1] = new Touch_Sensor(i, 30000, 60000);
        } else { // Ring [10, 13]
            touch_sensors[i-1] = new Touch_Sensor(i, 20000, 150000);
        }
        touch_sensors[i-1]->init();
    }

    touch_wheel = new Touch_Wheel(touch_sensors);

}

void loop() {   

    counter += touch_wheel->wheel();
    
    if (touch_wheel->button()) {
        swipe = "";
    }

    // Serial.println(touch_wheel->swipe());

    char c = touch_wheel->swipe();

    if (c != '\0') {
        swipe += c;
    }

    Serial.printf("Wheel: %02d      Swipe commands: %s\n", counter, swipe.c_str());
}
