#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN    6
#define SENSOR_PIN   8
#define STRIP_SIZE   68
#define PIX_PER_RUN  3

#define COLOR_ROT_TIME_MIN  75
#define COLOR_ROT_TIME_MAX  1275

#define R_COLOR_MIN      255
#define G_COLOR_MIN      0
#define B_COLOR_MIN      0
#define R_COLOR_MAX      0
#define G_COLOR_MAX      255
#define B_COLOR_MAX      100
#define R_RANGE          (double) (R_COLOR_MAX - R_COLOR_MIN)
#define G_RANGE          (double) (G_COLOR_MAX - G_COLOR_MIN)
#define B_RANGE          (double) (B_COLOR_MAX - B_COLOR_MIN)

#define RATE_ROT_TIME_MIN   51
#define RATE_ROT_TIME_MAX   2000
#define RATE_MIN         10
#define RATE_MAX         400
#define RATE_RANGE       (double) (RATE_MAX - RATE_MIN)

#define STOPPED          1275
#define R_STOPPED        255
#define G_STOPPED        0
#define B_STOPPED        0
#define STOPPED_RATE     400

class PixelThread: public Thread {
  public:
    Adafruit_NeoPixel strip;
    uint8_t pix_per_run;
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

    uint32_t rateToColor(uint16_t rate) {
      // red for low speeds, brighter and bluer/whiter as we go faster
      // max rate is 200 (lowest), min rate is 10
      // should spend some time looking at examples, thinking about this
      if (rate < 15) {
        rate = 15;
      }
      uint8_t r;
      uint8_t g;
      uint8_t b;
      if (rate >= 255) {
        r = 255;
        g, b = 0;
      } else {
        r = rate;
        g = 255 - rate;
        b = (uint8_t) ((r * g) / 75);
      }
      return strip.Color(r, g, b);
    }
};

class SpeedThread: public Thread {
  public:
    uint16_t rate;
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

  uint16_t timeToRate(long millisecs) {
    // possibilities:
    // no conversion (extremely slow)
    // divide by constant factor - based on rotations typically taking 200 - 1000ish ms,
    // if we divide by 10 we get 20 - 100 ms wait which seems about right
    // divide by constant factor with min and max
    // min shld probs be about 10ms, max should probs be abt 100-200ms
    long adjusted = (millisecs / 5);
    if (adjusted < 10) {
      return 10;
    }
    if (adjusted > 400) {
      return 400;
    }
    return (uint16_t) adjusted;
  }

  void setRate() {
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
  Serial.begin(9600);
}

void loop() {
  st.updateRotation();
  controller.run();
}
