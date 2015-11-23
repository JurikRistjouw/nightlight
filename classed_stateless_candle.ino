#include <Adafruit_NeoPixel.h>
#define LED_AMMOUNT 7
#define STRIP_BRIGHTNESS 128

// The onboard red LED's pin
#define REDLED_PIN              8
// The data-in pin of the NeoPixel
#define WICK_PIN                4
// Any unconnected pin, to try to generate a random seed
#define UNCONNECTED_PIN         2

// The LED can be in only one of these states at any given time
#define BRIGHT                  0
#define UP                      1
#define DOWN                    2
#define DIM                     3
#define BRIGHT_HOLD             4
#define DIM_HOLD                5

// Percent chance the LED will suddenly fall to minimum brightness
#define INDEX_BOTTOM_PERCENT    10
// Absolute minimum red value (green value is a function of red's value)
#define INDEX_BOTTOM            128
// Minimum red value during "normal" flickering (not a dramatic change)
#define INDEX_MIN               192
// Maximum red value
#define INDEX_MAX               255

// Decreasing brightness will take place over a number of milliseconds in this range
#define DOWN_MIN_MSECS          20
#define DOWN_MAX_MSECS          250
// Increasing brightness will take place over a number of milliseconds in this range
#define UP_MIN_MSECS            20
#define UP_MAX_MSECS            250
// Percent chance the color will hold unchanged after brightening
#define BRIGHT_HOLD_PERCENT     20
// When holding after brightening, hold for a number of milliseconds in this range
#define BRIGHT_HOLD_MIN_MSECS   0
#define BRIGHT_HOLD_MAX_MSECS   100
// Percent chance the color will hold unchanged after dimming
#define DIM_HOLD_PERCENT        5
// When holding after dimming, hold for a number of milliseconds in this range
#define DIM_HOLD_MIN_MSECS      0
#define DIM_HOLD_MAX_MSECS      50

#define MINVAL(A,B)             (((A) < (B)) ? (A) : (B))
#define MAXVAL(A,B)             (((A) > (B)) ? (A) : (B))


typedef unsigned char Byte;

class LedState
{
  public:
    LedState();
    void SetColor(byte index);
    void Update();


    byte red;
    byte green;
    byte blue;

  private:
    byte state;
    unsigned long flicker_msecs;
    unsigned long flicker_start;
    byte index_start;
    byte index_end;


};

LedState::LedState()
{
  SetColor(255);
  index_start = 255;
  index_end = 255;
  state = BRIGHT;
  return;
}

void LedState::SetColor(byte index)
{
  index = MAXVAL(MINVAL(index, INDEX_MAX), INDEX_BOTTOM);
  if (index >= INDEX_MIN)
  {
    green = index;
    red = (index * 3) / 8;
    blue = 0;
  }
  else if (index < INDEX_MIN)
  {
    green = index;
    red = (index * 3.25) / 8;
    blue = 0;
  }
  return;
}

void LedState::Update()
{
  unsigned long current_time = millis();

  switch (state)
  {
    case BRIGHT:
      flicker_msecs = random(DOWN_MAX_MSECS - DOWN_MIN_MSECS) + DOWN_MIN_MSECS;
      flicker_start = current_time;
      index_start = index_end;
      if ((index_start > INDEX_BOTTOM) &&
          (random(100) < INDEX_BOTTOM_PERCENT))
        index_end = random(index_start - INDEX_BOTTOM) + INDEX_BOTTOM;
      else
        index_end = random(index_start - INDEX_MIN) + INDEX_MIN;

      state = DOWN;
      break;
    case DIM:
      flicker_msecs = random(UP_MAX_MSECS - UP_MIN_MSECS) + UP_MIN_MSECS;
      flicker_start = current_time;
      index_start = index_end;
      index_end = random(INDEX_MAX - index_start) + INDEX_MIN;
      state = UP;
      break;
    case BRIGHT_HOLD:
    case DIM_HOLD:
      if (current_time >= (flicker_start + flicker_msecs))
        state = (state == BRIGHT_HOLD) ? BRIGHT : DIM;

      break;
    case UP:
    case DOWN:
      if (current_time < (flicker_start + flicker_msecs))
        SetColor(index_start + ((index_end - index_start) * (((current_time - flicker_start) * 1.0) / flicker_msecs)));
      else
      {
        SetColor(index_end);

        if (state == DOWN)
        {
          if (random(100) < DIM_HOLD_PERCENT)
          {
            flicker_start = current_time;
            flicker_msecs = random(DIM_HOLD_MAX_MSECS - DIM_HOLD_MIN_MSECS) + DIM_HOLD_MIN_MSECS;
            state = DIM_HOLD;
          }
          else
            state = DIM;
        }
        else
        {
          if (random(100) < BRIGHT_HOLD_PERCENT)
          {
            flicker_start = current_time;
            flicker_msecs = random(BRIGHT_HOLD_MAX_MSECS - BRIGHT_HOLD_MIN_MSECS) + BRIGHT_HOLD_MIN_MSECS;
            state = BRIGHT_HOLD;
          }
          else
            state = BRIGHT;
        }
      }
      
      break;
  }

  return;
}


//----------------------------

class Candle
{
  public:
    Candle();
    Candle(byte ammount);
    void Update();
    
  private:
    Adafruit_NeoPixel *strip;
    byte ledcount;
    LedState *ledstates;
};

Candle::Candle()
{
  Candle(2);
}

Candle::Candle(byte ammount)
{
  ledcount = ammount;
  ledstates = new LedState[ammount];
  
  randomSeed(analogRead(UNCONNECTED_PIN));
  // Turn off the onboard red LED
  pinMode(REDLED_PIN, OUTPUT);
  digitalWrite(REDLED_PIN, LOW);

  strip = new Adafruit_NeoPixel(ammount, WICK_PIN, NEO_RGB + NEO_KHZ800);
  strip->begin();
  strip->show();
  strip->setBrightness(STRIP_BRIGHTNESS);
//  strip->setPixelColor(0, 0, 255, 0);
//  strip->show();
  //delay(10);
  Update();
}

void Candle::Update()
{
  for (byte b=0; b < ledcount; b++)
  {
    ledstates[b].Update();
    strip->setPixelColor(b, ledstates[b].red, ledstates[b].green, ledstates[b].blue);
  }
  strip->show();
}


Candle *TheCandle;

void setup() {
  TheCandle = new Candle(LED_AMMOUNT);
  // put your setup code here, to run once:
}

void loop() {
  TheCandle->Update();
  // put your main code here, to run repeatedly:
}
