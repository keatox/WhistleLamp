#include <FastLED.h>
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioRealFFT.h"

// pin definition
#define SERVO 19
#define LED 18
#define NUM_LEDS 35               
#define WS 25  
#define SCK 33
#define SD 32

// LED ring bit data
CRGB leds[NUM_LEDS]; 

// audio config
uint16_t sample_rate = 44100;
uint8_t channels = 2;
I2SStream in;
AudioRealFFT fft;
StreamCopy copier(fft, in);

// global positioning variables
int servo = 2000;
int pwm;
int led = 0;   
int brightness;  
bool flip = true;   

void setup() {
  Serial.begin(115200);

  // initialize servo pin
  pinMode(SERVO, OUTPUT);

  // initialize LED ring
  FastLED.addLeds<WS2812, LED, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // I2S config
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

  // FFT algo config
  auto tcfg = fft.defaultConfig();
  tcfg.length = 8192;
  tcfg.channels = channels;
  tcfg.sample_rate = sample_rate;
  tcfg.bits_per_sample = 16;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);
}

void loop() {
  copier.copy(); // runs fft process
}

// FFT processing
void fftResult(AudioFFTBase &fft) {
  float diff;
  auto result = fft.result();

  // minimum audio volume threshold
  if (result.magnitude > 150) {
    if (flip) {
      Serial.print("servo ");
    } else {
      Serial.print("led ");
    }
    Serial.print(result.frequency);
    Serial.print(" => ");
    Serial.println(result.frequencyAsNote(diff));

    // adjust for frequencies within given range
    int increment = 100;
    if (result.frequency < 2000) {
      if (result.frequency > 1000) {
        // map value to given output
        if (flip) {
          servo = (result.frequency > 1500) ? min(servo + increment, 4095) : max(servo - increment, 0);
          pwm = constrain((500 + (servo * 2000) / 4095), 500, 2500);
          servoControl(pwm);
        } else {
          led = (result.frequency > 1500) ? min(led + increment, 4095) : max(led - increment, 0);
          brightness = (led * 255) / 4095;
          ledControl(brightness);
        }
      // flips output mode
      } else if (result.frequency > 800) {
        flip = !flip;
        flipNotify();
      }
    }
  }
}

// servo control
void servoControl(int pwm) {
  digitalWrite(SERVO, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(SERVO, LOW);
  delayMicroseconds((20000 - pwm) / 2);
  delayMicroseconds((20000 - pwm) / 2);
  delay(50);
}

// LED control
void ledControl(int brightness) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 0, brightness);  
  }
  FastLED.show();
}

// uses LED to notify output change
void flipNotify() {
  // runs a circular light loop across LED ring
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(0, 0, 150);  
      FastLED.show();
      delay(10);
      leds[i] = CHSV(0, 0, 0);
      FastLED.show();
    }
    ledControl(brightness);
}

