// Based off of Adafruit's NeoPixel strandtest_nodelay library example and Arduino's Arduino Uno R4 IntroUnoR4 sketch
// with my own changes and additions.

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "main.h"

#define LED_PIN    6
#define LED_COUNT 32

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long pixelPrevious = 0;        // Previous Pixel Millis
unsigned long patternPrevious = 0;      // Previous Pattern Millis
int           patternCurrent = 0;       // Current Pattern Number
int           patternInterval = 5000;   // Pattern Interval (ms)
bool          patternComplete = false;
int           pixelInterval = 50;       // Pixel Interval (ms)
int           pixelQueue = 0;           // Pattern Pixel Queue
int           pixelCycle = 0;           // Pattern Pixel Cycle
uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels

// setup() function -- runs once at startup --------------------------------
void setup() {
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

// loop() function -- runs repeatedly as long as board is on ---------------
void loop() {
    unsigned long currentMillis = millis();                     //  Update current time
    if( patternComplete || (currentMillis - patternPrevious) >= patternInterval) {  //  Check for expired time
        patternComplete = false;
        patternPrevious = currentMillis;
        patternCurrent++;                                         //  Advance to next pattern
        if(patternCurrent >= 1)
            patternCurrent = 0;
    }

    if(currentMillis - pixelPrevious >= pixelInterval) {        //  Check for expired time
        pixelPrevious = currentMillis;                            //  Run current frame
        switch (patternCurrent) {
            case 0:
                christmasChase(strip.Color(127, 0, 0), strip.Color(0, 127, 0), 150); // Christmas Lights
                break;
        }
    }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void christmasChase(uint32_t color, uint32_t color2, int wait) {
    static uint32_t loop_count = 0;
    static int16_t current_pixel = 0;
    static int16_t last_pixel = -1;
    const int LED_SPACING = 2;

    pixelInterval = wait;                   //  Update delay time

    strip.clear();

    for(int c=last_pixel; c < pixelNumber; c += LED_SPACING) {
        strip.setPixelColor(c, color2);
    }

    for(int c=current_pixel; c < pixelNumber; c += LED_SPACING) {
        strip.setPixelColor(c, color);
    }
    strip.show();

    current_pixel++;
    last_pixel++;
    if (current_pixel >= LED_SPACING) {
        current_pixel = 0;
        last_pixel = -1;
        loop_count++;
    }

    if (loop_count >= 10) {
        current_pixel = 0;
        loop_count = 0;
        patternComplete = true;
    }
}
