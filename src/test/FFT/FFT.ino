#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioRealFFT.h"

uint16_t sample_rate=44100;
uint8_t channels = 2; 
I2SStream in;
AudioRealFFT fft;
StreamCopy copier(fft, in);

void fftResult(AudioFFTBase &fft)
{
    float diff;
    auto result = fft.result();
    if (result.magnitude>100)
    {
        Serial.print(result.frequency);
        Serial.print(" => ");
        Serial.print(result.frequencyAsNote(diff));
    }
}

void setup(void) 
{ 
  Serial.begin(115200);
  
  auto configin = in.defaultConfig(RX_MODE);
  configin.sample_rate = sample_rate; 
  configin.channels = channels;
  configin.bits_per_sample = 16;
  configin.i2s_format = I2S_STD_FORMAT;
  configin.is_master = true;
  configin.port_no = 0;
  configin.pin_ws = 25;                             // ws (LRC) pin
  configin.pin_bck = 33;                            //BCLK pin
  configin.pin_data = 32;                        // SDOUT
  configin.pin_mck = 0;
  in.begin(configin); 

  auto tcfg = fft.defaultConfig();
  tcfg.length = 8192;
  tcfg.channels = channels;
  tcfg.sample_rate = sample_rate;
  tcfg.bits_per_sample = 16;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);
}

void loop() 
{
  copier.copy();
}
