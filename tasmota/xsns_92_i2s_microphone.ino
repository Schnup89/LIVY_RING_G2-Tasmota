/*
  xsns_92_i2s_microphone - PDM Mic Noise Detection support for Tasmota

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_I2S_MICROPHONE

#include "driver/i2s.h"
#include "arduinoFFT.h"

/*********************************************************************************************\
 * LivyRingG2 - PDM Mikrofon Noice Detection
 * 
 * References:
 * - https://iotassistant.io/esp32/smart-door-bell-noise-meter-using-fft-esp32/
 * 
 * 
 * TODO: 
 * FFT Library location?
 * Push Request 
 * 
\*********************************************************************************************/

#define XSNS_92                   92

#define SAMPLES                   1024

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = SAMPLES;

#define PYQ1548_AlertDBDiff       20    //Difference in db wher trigger sensorupdate
#define OCTAVES 9

// our FFT data
static float real[SAMPLES];
static float imag[SAMPLES];
static arduinoFFT fft(real, imag, SAMPLES, SAMPLES);
static float energy[OCTAVES];
static const float aweighting[] = {-39.4, -26.2, -16.1, -8.6, -3.2, 0.0, 1.2, 1.0, -1.1};
bool initdone = false;
float loudness = 0;



/*********************************************************************************************\
 * Helper-Functions
\*********************************************************************************************/
// calculates energy from Re and Im parts and places it back in the Re part (Im part is zeroed)
static void calculateEnergy(float *vReal, float *vImag, uint16_t samples)
{
    for (uint16_t i = 0; i < samples; i++)
    {
        vReal[i] = sq(vReal[i]) + sq(vImag[i]);
        vImag[i] = 0.0;
    }
}
// sums up energy in bins per octave
static void sumEnergy(const float *bins, float *energies, int bin_size, int num_octaves)
{
    // skip the first bin
    int bin = bin_size;
    for (int octave = 0; octave < num_octaves; octave++)
    {
        float sum = 0.0;
        for (int i = 0; i < bin_size; i++)
        {
            sum += real[bin++];
        }
        energies[octave] = sum;
        bin_size *= 2;
    }
}
// get value in decibel
static float decibel(float v)
{
    return 10.0 * log(v) / log(10);
}
// converts energy to logaritmic, returns A-weighted sum
static float calculateLoudness(float *energies, const float *weights, int num_octaves, float scale)
{
    float sum = 0.0;
    for (int i = 0; i < num_octaves; i++)
    {
        float energy = scale * energies[i];
        sum += energy * pow(10, weights[i] / 10.0);
        energies[i] = decibel(energy);
    }
    return decibel(sum);
}
// integer to floar helper function
static void integerToFloat(int32_t *integer, float *vReal, float *vImag, uint16_t samples)
{
    for (uint16_t i = 0; i < samples; i++)
    {
        vReal[i] = (integer[i] >> 16) / 10.0;
        vImag[i] = 0.0;
    }
}


/*********************************************************************************************\
 * Main-Functions
\*********************************************************************************************/
bool I2S_MICROInit(void)
{
  if (PinUsed(GPIO_I2S_IN_DATA) && PinUsed(GPIO_I2S_IN_SLCT))  // Only start, if the pins are configured
  {
    esp_err_t errtmp;

    i2s_config_t i2s_config = {
      mode: i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
      sample_rate: 22627,
      bits_per_sample: I2S_BITS_PER_SAMPLE_32BIT,
      channel_format: I2S_CHANNEL_FMT_ONLY_LEFT,
      communication_format: (i2s_comm_format_t)I2S_COMM_FORMAT_PCM,
      intr_alloc_flags: ESP_INTR_FLAG_LEVEL1,
      dma_buf_count: 8,
      dma_buf_len: BLOCK_SIZE,
      use_apll: false
    };
    i2s_pin_config_t pin_config = {
      bck_io_num:   -1, // not used in PDM-Mode, CLK is over WS(SLCT) PIN  
      ws_io_num:    Pin(GPIO_I2S_IN_SLCT),  
      data_out_num: -1, // not used
      data_in_num:  Pin(GPIO_I2S_IN_DATA),
    };

    errtmp = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL); 
    if (errtmp != ESP_OK) {
      AddLog(LOG_LEVEL_DEBUG,"I2S-Microphone: Failed install driver! %d", errtmp);
      return false;
    }
    #if (MIC_TIMING_SHIFT > 0) 
      // Undocumented (?!) manipulation of I2S peripheral registers
      // to fix MSB timing issues with some I2S microphones
      REG_SET_BIT(I2S_TIMING_REG(I2S_PORT), BIT(9));   
      REG_SET_BIT(I2S_CONF_REG(I2S_PORT), I2S_RX_MSB_SHIFT);  
    #endif
    errtmp = i2s_set_pin(I2S_PORT, &pin_config);
    if (errtmp != ESP_OK) {
      AddLog(LOG_LEVEL_DEBUG,"I2S-Microphone: Failed set PIN! %d", errtmp);
      return false;
    }
    AddLog(LOG_LEVEL_DEBUG,"I2S-Microphone: INIT OK!");
    initdone = true;
    return true;
  }else {
    return false;
  }
}

void I2S_MICROEverySecond(void) {
  if (initdone && PinUsed(GPIO_I2S_IN_DATA) && PinUsed(GPIO_I2S_IN_SLCT)) {  // Only start, if the pins are configured
    static int32_t samples[BLOCK_SIZE];
    float loudness_old = loudness;
    size_t num_bytes_read;
    esp_err_t err = i2s_read(I2S_PORT,
                             (char *)samples,
                             BLOCK_SIZE, // the doc says bytes, but its elements.
                             &num_bytes_read,
                             portMAX_DELAY); // no timeout
    int samples_read = num_bytes_read / 8;
     // integer to float
    integerToFloat(samples, real, imag, SAMPLES);
    // apply flat top window, optimal for energy calculations
    fft.Windowing(FFT_WIN_TYP_FLT_TOP, FFT_FORWARD);
    fft.Compute(FFT_FORWARD);
    // calculate energy in each bin
    calculateEnergy(real, imag, SAMPLES);
    // sum up energy in bin for each octave
    sumEnergy(real, energy, 1, OCTAVES);
    // calculate loudness per octave + A weighted loudness
    loudness = calculateLoudness(energy, aweighting, OCTAVES, 1.0);
    //Microphone deactivated, 81db reported show 0db instead
    if ((int)loudness == 81 ) {    
      loudness = 0;
    }
    //Update Sensors if AlertDBDiff is reached
    float loudness_diff = loudness_old - loudness;
    if (loudness_diff > PYQ1548_AlertDBDiff || loudness_diff < -PYQ1548_AlertDBDiff) MqttPublishSensor();
    //AddLog(LOG_LEVEL_DEBUG,"I2S-Microphone: Loud: %d",(int)loudness);
  }
}

void I2S_MICROShow(bool json)
{  
  if (PinUsed(GPIO_I2S_IN_DATA) && PinUsed(GPIO_I2S_IN_SLCT)) {
    if (json) {
      ResponseAppend_P(PSTR(",\"MIC\":{\"Noise_Level (db)\": %.1f}"), loudness);
  #ifdef USE_WEBSERVER
    } else {
      WSContentSend_PD("{s}Microphone Noise {m}%.1f db{e}", "Loudness",loudness);
  #endif  // USE_WEBSERVER
    }
  }
  return;
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns92(uint8_t function)
{
  bool result = false;
  switch (function) {
    case FUNC_INIT:
      result = I2S_MICROInit();
      break;
    case FUNC_EVERY_SECOND:
        I2S_MICROEverySecond();
        break;
    case FUNC_JSON_APPEND:
      I2S_MICROShow(1);
      break;
#ifdef USE_WEBSERVER
    case FUNC_WEB_SENSOR:
      I2S_MICROShow(0);
      break;
#endif  // USE_WEBSERVER
    }
  return result;
}

#endif  // USE_PYQ1548

