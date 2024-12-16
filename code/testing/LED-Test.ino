#include <FastLED.h>
#include <EEPROM-Storage.h>

// Setup LEDs
#define LED_PIN 2
#define NUM_LEDS 15
CRGB leds[NUM_LEDS];
CRGB red, green, yellow, orange, purple, blue;

//set individual led colors here
CRGB *colArray[NUM_LEDS] = {&green, &green, &green, &green, &green, &green, &yellow, &yellow, &yellow, &orange, &orange, &orange, &red, &red, &red};

#define dimmer 1

void setColors()
{
  //define colors and apply dimmer
  green = CRGB(0,255*dimmer,0);
  yellow = CRGB(255*dimmer,150*dimmer,0);
  orange = CRGB(255*dimmer,50*dimmer,0);
  red = CRGB(255*dimmer,0,0);
  purple = CRGB(255*dimmer,0,255*dimmer);
  blue = CRGB(0,0,255*dimmer);
}

void setAllLEDs(CRGB col)
{
  FastLED.clear();
    for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = col;
      }
      FastLED.show();
}

void flashLEDs(CRGB OGcol, CRGB flashCol)
{
  FastLED.clear();
  delay(100);
  setAllLEDs(flashCol);
  delay(600);
  setAllLEDs(OGcol);
}


void waveLEDs()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = *colArray[i];
    FastLED.show();
    delay(50);
  }
  for (int i = NUM_LEDS-1; i >= 0; i--)
  {
    FastLED.clear();
    leds[i] = *colArray[i];
    FastLED.show();
    delay(50);
  }
}


void setup() {
  //init leds
  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);

  
  //init serial
  Serial.begin(9600);

  //setup colors
  setColors();

  waveLEDs();

  Serial.println("Setup Complete");


}

void loop() {
  waveLEDs();

  Serial.println("looping :)");
  
}