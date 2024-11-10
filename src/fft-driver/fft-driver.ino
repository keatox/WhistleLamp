#include <FastLED.h>
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioRealFFT.h"

#define SERVO 18
#define POT 34
#define LED 19
#define NUM_LEDS 35               
CRGB leds[NUM_LEDS]; 
#define WS 25  
#define SCK 33
#define SD 32

uint16_t sample_rate=44100;
uint8_t channels = 2; 
I2SStream in;
AudioRealFFT fft;
StreamCopy copier(fft, in);   
int pot = 0;        

void setup() {
  Serial.begin(115200);
  pinMode(SERVO, OUTPUT);
  pinMode(POT, INPUT);
  FastLED.addLeds<WS2812, LED, GRB>(leds, NUM_LEDS);  

  auto configin = in.defaultConfig(RX_MODE);
  configin.sample_rate = sample_rate; 
  configin.channels = channels;
  configin.bits_per_sample = 16;
  configin.i2s_format = I2S_STD_FORMAT;
  configin.is_master = true;
  configin.port_no = 0;
  configin.pin_ws = WS;                             
  configin.pin_bck = SCK;                            
  configin.pin_data = SD;                        
  configin.pin_mck = 0;
  in.begin(configin); 

  //
  // Configure FFT
  //
  auto tcfg = fft.defaultConfig();
  tcfg.length = 8192;
  tcfg.channels = channels;
  tcfg.sample_rate = sample_rate;
  tcfg.bits_per_sample = 16;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);
}

void loop() {
  copier.copy();
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

void fftResult(AudioFFTBase &fft) {
  float diff;
  auto result = fft.result();

  if (result.magnitude>100)
  {
      Serial.print(result.frequency);
      Serial.print(" => ");
      Serial.println(result.frequencyAsNote(diff));
  }

  int increment = 50;
  if (result.frequency > 100) {
    if (result.frequency > 1200) {
      pot = (pot < 4095 - increment) ? pot + increment : 4095;
    } else {
      pot = (pot > increment) ? pot - increment : 0;
    }
  }

  int pwm = (500 + (pot * 2000) / 4095);
  int brightness = (pot * 255) / 4095;
  servo(pwm);
  led(brightness);
}
