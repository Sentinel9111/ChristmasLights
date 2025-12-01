#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LED>

#define LED_PIN     6
#define LED_COUNT   32

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------------------------------------------------------------------------
// Timing & Pattern State
// ---------------------------------------------------------------------------
unsigned long pixelPrevious = 0;
unsigned long patternPrevious = 0;

int patternCurrent = 0;
int patternInterval = 5000;
bool patternComplete = false;

int pixelInterval = 50;
uint16_t pixelNumber = LED_COUNT;

// ---------------------------------------------------------------------------
// Twinkle System
// ---------------------------------------------------------------------------
struct Twinkle {
    int pixel;
    uint8_t brightness;
    bool active;
};

const int MAX_TWINKLES = 6;
Twinkle twinkles[MAX_TWINKLES];

void spawnTwinkle() {
    for (int i = 0; i < MAX_TWINKLES; i++) {
        if (!twinkles[i].active) {
            twinkles[i].pixel = random(pixelNumber);
            twinkles[i].brightness = 127;
            twinkles[i].active = true;
            return;
        }
    }
}

void updateTwinkles() {
    for (int i = 0; i < MAX_TWINKLES; i++) {
        if (twinkles[i].active) {
            // Fade sparkle
            if (twinkles[i].brightness > 15)
                twinkles[i].brightness -= 8;
            else {
                twinkles[i].active = false;
                continue;
            }

            uint8_t b = twinkles[i].brightness;
            strip.setPixelColor(twinkles[i].pixel, strip.Color(b, b, b));
        }
    }
}

// ---------------------------------------------------------------------------
// Color helper with brightness scaling
// ---------------------------------------------------------------------------
uint32_t scaleColor(uint32_t color, float brightness) {
    uint8_t r = (uint8_t)((color >> 16 & 0xFF) * brightness);
    uint8_t g = (uint8_t)((color >> 8 & 0xFF) * brightness);
    uint8_t b = (uint8_t)((color & 0xFF) * brightness);
    return strip.Color(r, g, b);
}

// ---------------------------------------------------------------------------
// Theater Chase with smooth red/green fade
// ---------------------------------------------------------------------------
float leadBrightness = 1.0;     // 1.0 = red lead, 0.0 = green lead
float leadStep = 0.02;          // amount to change per frame
bool fadingDown = true;          // true = fading towards green

void theaterChase(uint32_t redColor, uint32_t greenColor, int wait) {
    static uint32_t loop_count = 0;
    static uint16_t current_pixel = 0;
    static int16_t last_pixel = -1;
    const int LED_SPACING = 2;

    pixelInterval = wait;

    // Update lead brightness for smooth fade
    if (fadingDown) {
        leadBrightness -= leadStep;
        if (leadBrightness <= 0.0) {
            leadBrightness = 0.0;
            fadingDown = false;
        }
    } else {
        leadBrightness += leadStep;
        if (leadBrightness >= 1.0) {
            leadBrightness = 1.0;
            fadingDown = true;
        }
    }

    // Compute scaled colors
    uint32_t redPixel   = scaleColor(redColor, leadBrightness);
    uint32_t greenPixel = scaleColor(greenColor, 1.0 - leadBrightness);

    strip.clear();

    // Draw second (dim) color row
    for (int c = last_pixel; c < pixelNumber; c += LED_SPACING) {
        if (c >= 0) strip.setPixelColor(c, greenPixel);
    }

    // Draw first (bright) color row
    for (int c = current_pixel; c < pixelNumber; c += LED_SPACING) {
        strip.setPixelColor(c, redPixel);
    }

    // Overlay sparkles
    updateTwinkles();

    strip.show();

    // Advance chase
    current_pixel++;
    last_pixel++;
    if (current_pixel >= LED_SPACING) {
        current_pixel = 0;
        last_pixel = -1;
        loop_count++;
    }

    if (loop_count >= 10) {
        loop_count = 0;
        current_pixel = 0;
        patternComplete = true;
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
    strip.begin();
    strip.show();
    strip.setBrightness(80);       // overall brightness
    randomSeed(analogRead(0));
}

// ---------------------------------------------------------------------------
// Main Loop
// ---------------------------------------------------------------------------
void loop() {
    unsigned long currentMillis = millis();

    // Pattern timeout
    if (patternComplete || (currentMillis - patternPrevious >= patternInterval)) {
        patternComplete = false;
        patternPrevious = currentMillis;
        patternCurrent++;
        if (patternCurrent >= 1)
            patternCurrent = 0;
    }

    // Frame update
    if (currentMillis - pixelPrevious >= pixelInterval) {
        pixelPrevious = currentMillis;

        // Spawn random sparkles
        if (random(100) < 10) {
            spawnTwinkle();
        }

        switch (patternCurrent) {
            case 0:
                theaterChase(
                    strip.Color(127, 0, 0),   // red
                    strip.Color(0, 127, 0),   // green
                    200                        // chase speed
                );
                break;
        }
    }
}
