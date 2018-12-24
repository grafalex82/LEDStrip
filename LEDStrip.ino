#include <WS2812FX.h>

#define LED_COUNT 50
#define LED_PIN 6

#define BTN_PIN 2

#define TIMER_MS 20000
#define DEBOUNCE_MS 30
#define LONG_PRESS_MS 2000

enum ButtonStates
{
  RELEASED,
  DEBOUNCING,
  PRESSED
};

ButtonStates btnState = RELEASED;

enum ButtonPressType
{
  NONE,
  SHORT_PRESS,
  LONG_PRESS
};

// List of modes I like (sorted randomly)
const unsigned char AllowedModes[] = {
  FX_MODE_CHASE_RAINBOW,
  FX_MODE_CUSTOM,
  FX_MODE_RAINBOW_CYCLE,
  FX_MODE_RANDOM_COLOR,
  FX_MODE_COLOR_WIPE_REV,
  FX_MODE_LARSON_SCANNER,
  FX_MODE_CHASE_COLOR,
  FX_MODE_COMET,
  FX_MODE_FADE,
  FX_MODE_CHASE_WHITE,
  FX_MODE_COLOR_SWEEP_RANDOM,
  FX_MODE_CHASE_RANDOM,
  FX_MODE_COLOR_WIPE_RANDOM,
  FX_MODE_CHASE_BLACKOUT_RAINBOW,
  FX_MODE_MULTI_DYNAMIC,
  FX_MODE_SCAN,
  FX_MODE_RAINBOW,
  FX_MODE_COLOR_WIPE_INV,
  FX_MODE_BREATH,
  FX_MODE_BLINK,
  FX_MODE_DUAL_SCAN,
  FX_MODE_SINGLE_DYNAMIC,
  FX_MODE_COLOR_WIPE,
  FX_MODE_CHASE_RAINBOW_WHITE,
  FX_MODE_COLOR_WIPE_REV_INV,
  FX_MODE_RUNNING_LIGHTS,
  FX_MODE_BLINK_RAINBOW
};


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

unsigned long last_change = 0;
unsigned long now = 0;
bool disableSwitchOnTimer = false;
int curMode = 0;

void switchMode()
{
  ws2812fx.setColor(random()%255, random()%255, random()%255);

  curMode = (curMode + 1) % (sizeof(AllowedModes) / sizeof(AllowedModes[0]));
  ws2812fx.setMode(AllowedModes[curMode]);

  last_change = now;

  Serial.print("Switched to mode ");
  Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
}

uint16_t myCustomEffect(void)
{ // random chase
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  for(uint16_t i=seg->stop; i>seg->start; i--) {
    ws2812fx.setPixelColor(i, ws2812fx.getPixelColor(i-1));
  }
  uint32_t color = ws2812fx.getPixelColor(seg->start + 1);
  int r = random(6) != 0 ? (color >> 16 & 0xFF) : random(256);
  int g = random(6) != 0 ? (color >> 8  & 0xFF) : random(256);
  int b = random(6) != 0 ? (color       & 0xFF) : random(256);
  ws2812fx.setPixelColor(seg->start, r, g, b);
  return seg->speed / 16; // return the delay until the next animation step (in msec)
}

bool btnPressed()
{
  return digitalRead(BTN_PIN) == LOW;
}

unsigned long btnPressTime = 0;

ButtonPressType getButtonPressType()
{
  // Caught first press
  if(btnState == RELEASED && btnPressed())
  {
    btnState = DEBOUNCING;
    btnPressTime = millis();
    return NONE;
  }

  unsigned long duration = millis() - btnPressTime;
  if(btnState == DEBOUNCING && duration > DEBOUNCE_MS)
  {
    btnState = PRESSED;
    return NONE;
  }

  if(btnState == PRESSED && !btnPressed())
  {
    btnState = RELEASED;
    if(duration > LONG_PRESS_MS)
      return LONG_PRESS;
    else
      return SHORT_PRESS;
  }

  return NONE;
}

void setup()
{
  Serial.begin(115200);

  pinMode(BTN_PIN, INPUT_PULLUP);
  
  ws2812fx.init();
  ws2812fx.setBrightness(200);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  
  ws2812fx.setCustomMode(myCustomEffect);
  
  ws2812fx.start();
}

void loop() {
  now = millis();

  ws2812fx.service();

  if(now - last_change > TIMER_MS && !disableSwitchOnTimer)
  {
    Serial.println("Switching mode by timer");
    switchMode();
  }

  if(Serial.available())
  {
    while(Serial.available())
      Serial.read();

    Serial.println("Switching mode by serial");
    switchMode();
  }

  ButtonPressType btnPressType = getButtonPressType();
  if(btnPressType == SHORT_PRESS)
  {
    Serial.println("Switching mode with button");
    disableSwitchOnTimer = false;
    switchMode();
  }

  if(btnPressType == LONG_PRESS)
  {
    Serial.println("Disabling switching mode on timer");
    disableSwitchOnTimer = true;
  }
}

