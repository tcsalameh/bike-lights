/*
This code will animate a "rainbow" style of light display.
The rainbow will start all around light strip, and then
each pixel will take the color of the pixel ahead of it
when movement is detected. This creates a continuous stock-ticker
like display.

The rate at which the lights move forward changes depending on how fast the
wheel is rotating. Maximum and minimum values can be configured.

The color of the lights also changes depending on speed, and maximum and
minimum colors can be selected. The lights will step through a smooth range
of intermediate colors as speed increases until they reach the maximum.
*/

#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN    6
#define SENSOR_PIN   8
#define STRIP_SIZE   68      // number of pixels in light strip
#define PIX_PER_COLOR 6      // number of pixels per color band

#define RATE_ROT_TIME_MIN   51     // milliseconds between wheel rotations below which we're at the fastest change rate
#define RATE_ROT_TIME_MAX   4000   // milliseconds between wheel rotations above which we're at the slowest change rate
#define RATE_MIN            10     // minimum number of milliseconds between transitions
#define RATE_MAX            400    // maximum number of milliseconds between transitions
#define RATE_RANGE          (double) (RATE_MAX - RATE_MIN)    // total range of transition rate values

#define STOPPED          2000    // if wheel rotation takes more milliseconds than this, we consider the bike stopped
#define STOPPED_RATE     400     // milliseconds between animation transition when we're stopped

#define N_COLORS 6

class PixelThread: public Thread {
/* Class to encapsulate the thread controlling a light strip */
  public:
    Adafruit_NeoPixel strip;
    uint16_t offset;
    bool on;

    PixelThread(
      Adafruit_NeoPixel _strip,
      uint16_t _offset
    ) {
      strip = _strip;
      offset = _offset;
      on = false;
    }

    void toggleOnState() {
      on = !on;
    }

    void updateOffset() {
      if (offset >= STRIP_SIZE - 1) {
        offset = 0;
      } else {
        offset += 1;
      }
    }
};

class SpeedThread: public Thread {
/* Class to encapsulate a thread that measures the time of each wheel rotation */
  public:
    uint16_t rate;    // the update rate we will calculate for the animation
    long rotation_time;

    long _timeOfLastHit;
    long _timeSinceLastRotation;
    bool _reset;

  SpeedThread() {
    rate = 300;
    rotation_time = 4000;
    _timeSinceLastRotation = 5000;
    _timeOfLastHit = 0;
    _reset = false;
  }

  void updateRotation() {
  /* updates the time since the last rotation,
  should be called in a tight loop */
    if (_reset) {
      _reset = (digitalRead(SENSOR_PIN) == LOW);
      return;
    }
    if ((digitalRead(SENSOR_PIN) == LOW) && (millis() - _timeOfLastHit > 100)) {
      _timeSinceLastRotation = millis() - _timeOfLastHit;
      _timeOfLastHit = millis();
      _reset = true;
    }
  }

  uint16_t calculateRate() {
  /* Based on the milliseconds it takes the wheel to rotate,
  calculates the number of milliseconds between animation updates.
  It maps the rotation time linearly against the min and max rates. */
    double pctg;

    if (rotation_time >= STOPPED)
      return STOPPED_RATE;

    pctg = abs(rotation_time - RATE_ROT_TIME_MIN) / (double) (RATE_ROT_TIME_MAX - RATE_ROT_TIME_MIN);
    if (pctg < 0)
      pctg = 0.0;
    if (pctg > 1)
      pctg = 1.0;

    return RATE_MIN + (uint16_t) (pctg * RATE_RANGE);
  }

  void setRate() {
  /* callback to periodically calculate rate, should be called
  more frequently than the fastest rotation time */
    rotation_time = max(_timeSinceLastRotation, millis() - _timeOfLastHit);
    rate = calculateRate();
  }
};

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_SIZE, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
PixelThread pt = PixelThread(strip, (uint16_t) 0);
SpeedThread st = SpeedThread();

ThreadController controller = ThreadController();

const uint32_t colors[] = {
  // red
  strip.Color(231, 0, 0),
  // orange
  strip.Color(255, 140, 0),
  // yellow
  strip.Color(255, 239, 0),
  // green
  strip.Color(0, 239, 31),
  // blue
  strip.Color(0, 68, 255),
  //purple
  strip.Color(118, 0, 137)
};

void initRainbow() {
  for (uint16_t i=0; i < STRIP_SIZE; i+=1) {
    strip.setPixelColor(i, calculateRainbow(i, 0));
  }
  pt.toggleOnState();
  strip.show();
}

uint32_t calculateRainbow(uint16_t index, uint16_t offset) {
  // Translate where the strip is now in its rotation
  // back onto where it started to make a mapping.
  uint16_t offsetIndex;
  if (offset <= index) {
    offsetIndex = index - offset;
  } else {
    offsetIndex = 68 + index - offset;
  }
  return colors[(offsetIndex / PIX_PER_COLOR) % N_COLORS];
}

void speedCallback() {
  st.setRate();
}

void lightStripCallback() {
/* Callback to update the light strip.
 * When not stopped, fills in the pixel ahead with the color of the pixel behind. Since its
 * slow to query the lightstrip for it, we calculate directly using the offeset.
 */
   if (st.rate >= STOPPED_RATE) {
     pt.toggleOnState();
     if (pt.on) {
       for (uint16_t pixNum = 0; pixNum < STRIP_SIZE; pixNum++) {
         strip.setPixelColor(pixNum, 0);
       }
     } else {
         for (uint16_t pixNum = 0; pixNum < STRIP_SIZE; pixNum++) {
           strip.setPixelColor(pixNum, calculateRainbow(pixNum, pt.offset));
         }
     }

     strip.show();
     return;
   }
   
   for (uint16_t pixNum = 0; pixNum < STRIP_SIZE; pixNum++) {
     strip.setPixelColor(pixNum, calculateRainbow(pixNum, pt.offset));
   }
  // set it to the color of the one behind it

  strip.show();
  
  pt.updateOffset();
  pt.setInterval(st.rate);
}


void setup() {
  pinMode(SENSOR_PIN, INPUT);
  strip.begin();
  strip.show();
  initRainbow();

  pt.onRun(lightStripCallback);
  pt.setInterval(50);

  st.onRun(speedCallback);
  st.setInterval(201);

  controller.add(&pt);
  controller.add(&st);
}

void loop() {
  // Rather than explicitly calling as another pseudo-thread,
  // found it more effective to recalculate rotation rate on every loop.
  st.updateRotation();
  controller.run();
}
