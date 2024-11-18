#include <FastLED.h>
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioRealFFT.h"

// Pin Definitions
#define SERVO 19
#define LED 18
#define NUM_LEDS 35               
CRGB leds[NUM_LEDS]; 
#define WS 25  
#define SCK 33
#define SD 32

// Audio Configuration
uint16_t sample_rate = 44100;
uint8_t channels = 2;
I2SStream in;
AudioRealFFT fft;
StreamCopy copier(fft, in);

// Variables
int pot = 0;        

void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Initialize Pins
  pinMode(SERVO, OUTPUT);

  // Initialize LEDs
  FastLED.addLeds<WS2812, LED, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // Configure I2S Input
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
  
  if (!in.begin(configin)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // Halt for debugging
  }

  // Configure FFT
  auto tcfg = fft.defaultConfig();
  tcfg.length = 8192;
  tcfg.channels = channels;
  tcfg.sample_rate = sample_rate;
  tcfg.bits_per_sample = 16;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);
}

void loop() {
  copier.copy(); // Continuously process audio
}

// Servo Control Function
void servo(int pwm) {
  digitalWrite(SERVO, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(SERVO, LOW);
  delayMicroseconds((20000 - pwm) / 2);
  delayMicroseconds((20000 - pwm) / 2);
  delay(50);
}

// LED Control Function
void led(int brightness) {
  static int prevBrightness = -1;
  if (brightness != prevBrightness) {
    prevBrightness = brightness;
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(0, 0, brightness);  
    }
    FastLED.show();
  }
}

// FFT Result Processing
void fftResult(AudioFFTBase &fft) {
  float diff;
  auto result = fft.result();

  // Only process significant magnitudes
  if (result.magnitude > 10) {
    Serial.print(result.frequency);
    Serial.print(" => ");
    Serial.println(result.frequencyAsNote(diff));

    // Adjust pot value based on frequency range
    int increment = 100;
    if (result.frequency > 500 && result.frequency < 2000) {
      pot = (result.frequency > 1400) ? min(pot + increment, 4095)
                                     : max(pot - increment, 0);
    }

    // Map pot value to servo PWM and LED brightness
    int pwm = constrain((500 + (pot * 2000) / 4095), 500, 2500);
    int brightness = (pot * 255) / 4095;

    // Update servo and LEDs
    servo(pwm);
    led(brightness);
  }
}
