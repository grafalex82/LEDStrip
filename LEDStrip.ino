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

void switchMode()
{
  ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
  last_change = now;

  Serial.print("Switched to mode ");
  Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
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
