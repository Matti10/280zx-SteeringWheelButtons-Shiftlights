#include <FastLED.h>
#include <EEPROM-Storage.h>

// Setup LEDs
#define LED_PIN 2
#define NUM_LEDS 15
CRGB leds[NUM_LEDS];
CRGB red, green, yellow, orange, purple, blue;

//set individual led colors here
CRGB *colArray[NUM_LEDS] = {&green, &green, &green, &green, &green, &green, &yellow, &yellow, &yellow, &orange, &orange, &orange, &red, &red, &red};

//setup buttons
#define NUM_BUTTONS 5
#define HOLD_TIME 500 //1000 is roughly equivilent to 5secs
#define CONF_BUT_0 0
#define CONF_BUT_1 1
int buttonPins[NUM_BUTTONS] = {3,4,5,6,7};
int buttonStates[NUM_BUTTONS] = {LOW, LOW, LOW, LOW, LOW};


// Setup RPM
#define RPM_PIN A5
#define REDLINE_RPM 0.95
#define MAX_RPM_ADDR 0
#define MIN_RPM_ADDR 1
double MAX_RPM;
double MIN_RPM;
double RPM_STEP = ((MAX_RPM - MIN_RPM) / NUM_LEDS);

double rpm = 0;

// Setup Dimmer
#define DIM_PIN A4
#define MAX_DIM_ADDR 2
#define MIN_DIM_ADDR 3
double MAX_DIM;
double MIN_DIM;



//setup states
enum states {main, dimConfig, rpmMinConfig , rpmMaxConfig, save};
enum states state = main;


double volt_dim_ratio = 1;
double dimmer = 0.25;

//converts a value to a percentage in a given range
double tooPercent(double min, double max, double val) {
 return (val-min)/(max-min);
}

void nextState()
{
  if (state == main)
  {
    state = dimConfig;
  }
  else if (state == dimConfig)
  {
    state = rpmMinConfig;
  }
  else if (state == rpmMinConfig)
  {
    state = rpmMaxConfig;
  }
  else if (state == rpmMaxConfig)
  {
    state = save;
  }
  else
  {
    state = main;
  }
}

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

void clearButtns()
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttonStates[i] = 0;
  }
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

  //init inputs
  pinMode(RPM_PIN,INPUT);//rpm
  pinMode(DIM_PIN,INPUT);//dimmer
  //buttons
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(buttonPins[i],INPUT);
  }

  //init serial
  Serial.begin(9600);

  //setup colors
  setColors();

  //init non-volatile mem and read the values
  MAX_RPM = EEPROM.read(MAX_RPM_ADDR);
  MIN_RPM = EEPROM.read(MIN_RPM_ADDR);
  MAX_DIM = EEPROM.read(MAX_DIM_ADDR);
  MIN_DIM = EEPROM.read(MIN_DIM_ADDR);

  waveLEDs();
}

void loop() {

  Serial.println(state);
  //check button states
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    buttonStates[i] += digitalRead(buttonPins[i]);
  }
  //check for state change
  if (buttonStates[CONF_BUT_0] > HOLD_TIME && buttonStates[CONF_BUT_1] > HOLD_TIME)
  {
    clearButtns();
    nextState();
  }

  switch (state)
  {
    case dimConfig:
      setAllLEDs(purple);
      dimmer = analogRead(DIM_PIN); //read dimmer value
      
      //reset max
      if (buttonStates[CONF_BUT_0] > HOLD_TIME*2)
      {
        MAX_DIM = 0;
        flashLEDs(purple,yellow);
        clearButtns();
      }

      //reset min
      if (buttonStates[CONF_BUT_1] > HOLD_TIME*2)
      {
        MIN_DIM = 1023;
        flashLEDs(purple,yellow);       
        clearButtns();
      }

      //if the value is higher than the current Max update max accordinly, and vice versa
      if (dimmer > MAX_DIM)
      {
        MAX_DIM = dimmer;
      }
      if (dimmer < MIN_DIM)
      {
        MIN_DIM = dimmer;
      }
      break;
    case rpmMinConfig:
      setAllLEDs(blue);

      //reset max
      if (buttonStates[CONF_BUT_0] > HOLD_TIME*2)
      {
        MIN_RPM = 0;
        flashLEDs(blue, yellow);
        clearButtns();
      }

      rpm = analogRead(RPM_PIN);

      if (rpm > MIN_RPM)
      {
        MIN_RPM = rpm;
      }
      break;
    case rpmMaxConfig:
      setAllLEDs(red);

      //reset min
      if (buttonStates[CONF_BUT_0] > HOLD_TIME*2)
      {
        MAX_RPM = 0;
        flashLEDs(red, yellow);
        clearButtns();
      }

      rpm = analogRead(RPM_PIN);

      if (rpm > MAX_RPM)
      {
        MAX_RPM = rpm;
      }
      break;
    case save:
      setAllLEDs(orange);
      //save all
      EEPROM.write(MAX_RPM_ADDR,MAX_RPM);
      EEPROM.write(MIN_RPM_ADDR,MIN_RPM);
      EEPROM.write(MAX_DIM_ADDR,MAX_DIM);
      EEPROM.write(MAX_DIM_ADDR,MIN_DIM);

      state = main;
      break;
    default:
      //read RPM and convert to a percent
      // rpm = tooPercent(0,1023,analogRead(RPM_PIN));
      rpm = 0.00;

      // clear all LEDs
      FastLED.clear(true);

      if (rpm >= REDLINE_RPM)
      {
        setAllLEDs(red);      
        delay(100);
        FastLED.clear(true);
        delay(100);
      }
      else if (rpm > MIN_RPM)
      {
        
        for (int i = 0; i < NUM_LEDS; i++)
        {
          if (rpm > ((i * RPM_STEP)+MIN_RPM))
          {
            leds[i] = *colArray[i];
          }
        }

        FastLED.show();
      }
      else
      {
        //check dimmer level
        dimmer = tooPercent(0,1023,analogRead(DIM_PIN)*volt_dim_ratio);

        //re-apply dimmer to colors
        setColors();

      }
      break;
  }
  
}