#include <FastLED.h>

#define SERVO 18
#define POT 34
#define LED 19
#define NUM_LEDS 35               

CRGB leds[NUM_LEDS];              

void setup() {
  Serial.begin(115200);
  pinMode(SERVO, OUTPUT);
  pinMode(POT, INPUT);
  FastLED.addLeds<WS2812, LED, GRB>(leds, NUM_LEDS);  
}

void loop() {
  int pot = analogRead(POT);
  int pwm = (500 + (pot * 2000) / 4095) / 50 * 50;
  int brightness = (pot * 255) / 4095;
  servo(pwm);
  led(brightness);
}

void servo(int pwm) {
  digitalWrite(SERVO, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(SERVO, LOW);
  delayMicroseconds((20000 - pwm) / 2);
  delayMicroseconds((20000 - pwm) / 2);
  delay(50);
}

void led(int brightness) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 0, brightness);  
  }
  FastLED.show();
}
