/*
This code will animate a "caterpillar" style of light display.
Lights will start in a line of PIX_PER_RUN pixels, and "crawl" up
towards the headtube, starting again at the bottom of the downtube.

The rate at which the lights "crawl" changes depending on how fast the
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
#define STRIP_SIZE   68    // number of pixels in light strip
#define PIX_PER_RUN  3     // number of pixels in "caterpillar"

#define COLOR_ROT_TIME_MIN  75      // milliseconds between wheel rotations below which we're at the fastest color
#define COLOR_ROT_TIME_MAX  1275    // milliseconds between wheel rotations below which we're at teh slowest color

#define R_COLOR_MIN      255    // red value at slowest speed
#define G_COLOR_MIN      0      // green value at slowest speed
#define B_COLOR_MIN      0      // blue value at slowest speed
#define R_COLOR_MAX      0      // red value at fastest speed
#define G_COLOR_MAX      255    // green value at fastest speed
#define B_COLOR_MAX      100    // blue value at fastest speed
#define R_RANGE          (double) (R_COLOR_MAX - R_COLOR_MIN)    // total range of red values
#define G_RANGE          (double) (G_COLOR_MAX - G_COLOR_MIN)    // total range of green values
#define B_RANGE          (double) (B_COLOR_MAX - B_COLOR_MIN)    // total range of blue values

#define RATE_ROT_TIME_MIN   51     // milliseconds between wheel rotations below which we're at the fastest change rate
#define RATE_ROT_TIME_MAX   4000   // milliseconds between wheel rotations above which we're at the slowest change rate
#define RATE_MIN            10     // minimum number of milliseconds between transitions
#define RATE_MAX            400    // maximum number of milliseconds between transitions
#define RATE_RANGE          (double) (RATE_MAX - RATE_MIN)    // total range of transition rate values

#define STOPPED          2000    // if wheel rotation takes more milliseconds than this, we consider the bike stopped
#define R_STOPPED        255     // red value when we're stopped
#define G_STOPPED        0       // green value when we're stopped
#define B_STOPPED        0       // blue value when we're stopped
#define STOPPED_RATE     400     // milliseconds between animation transition when we're stopped

class PixelThread: public Thread {
/* Class to encapsulate the thread controlling a light strip */
  public:
    Adafruit_NeoPixel strip;
    uint8_t pix_per_run;    // number of pixels in "caterpillar"
    uint16_t index;
    uint32_t color;

    PixelThread(
      Adafruit_NeoPixel _strip,
      uint8_t _pix_per_run
    ) {
      strip = _strip;
      pix_per_run = _pix_per_run;
      index = 0;
      color = strip.Color(255, 0, 0);
    }

    uint32_t calculateColor(long rotation_time) {
      /* This function calculates what color the pixels
      should be based on the number of milliseconds one wheel
      revolution takes. It finds where the rotation time falls
      between our minimum and maximum rotation times, translates
      that into a percentage, and then uses the percentage to
      calculate how far between min and max each RGB component should
      lie. */
      double pctg;

      if (rotation_time >= STOPPED)
        return strip.Color(R_STOPPED, G_STOPPED, B_STOPPED);

      pctg = abs(rotation_time - COLOR_ROT_TIME_MAX) / (double) (COLOR_ROT_TIME_MAX - COLOR_ROT_TIME_MIN);
      if (pctg < 0)
        pctg = 0.0;
      if (pctg > 1)
        pctg = 1.0;

      return strip.Color(
          R_COLOR_MIN + (uint8_t) (pctg * R_RANGE),
          G_COLOR_MIN + (uint8_t) (pctg * G_RANGE),
          B_COLOR_MIN + (uint8_t) (pctg * B_RANGE)
        );
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
PixelThread pt = PixelThread(strip, PIX_PER_RUN);
SpeedThread st = SpeedThread();

ThreadController controller = ThreadController();

void speedCallback() {
  st.setRate();
}

void lightStripCallback() {
/* Callback to update the light strip.
Clears the last pixel in the "caterpillar",
and fills in the next pixel ahead. */
  uint16_t n = strip.numPixels();
  if (pt.index > 0) {
    strip.setPixelColor(pt.index-1, 0);
    strip.setPixelColor(n - pt.index, 0);
  }
  uint32_t c = pt.calculateColor(st.rotation_time);
  for (uint16_t j=pt.index; j<(pt.index + pt.pix_per_run); j++) {
    strip.setPixelColor(j, c);
  }
  for (uint16_t l=(n-pt.index-1); l>=(n-pt.index-pt.pix_per_run); l--) {
    strip.setPixelColor(l, c);
  }
  if (pt.index <= ((n / 2) - pt.pix_per_run)) {
    pt.index++;
  } else {
    for (uint16_t k=pt.index; k<(n / 2) - 1; k++) {
      strip.setPixelColor(k, 0);
    }
    for (uint16_t x=(n-pt.index-1); x>(n / 2); x--) {
      strip.setPixelColor(x, 0);
    }
    pt.index = 0;
  }
  strip.show();

  pt.setInterval(st.rate);
}


void setup() {
  pinMode(SENSOR_PIN, INPUT);
  strip.begin();
  strip.show();

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
