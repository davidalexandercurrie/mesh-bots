#include <Arduino.h>
#include <driver/dac.h>
#include "painlessMesh.h"
#include <ArduinoJson.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

int preset = 4;

int presetChanger = 0;

void receivedCallback(uint32_t from, String &msg);

painlessMesh mesh;

#define PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899
const int potPin = 34;
float potValue = 0;
bool up = true;
int count = 0;
double m = 0.0;
float vol = 0.1;
float SR = 1000000;
float p = 0;
float out = 0;
float mainVol = 0;
int level = 50;
bool playSineWave = false;
bool playSqrWave = false;
bool playSawWave = false;
bool playTriWave = false;
float freq = 100;
int wave = 0;
int presetTime = random(100000, 500000);

float sine(float freq, float volume)
{
  float SI = freq * 2 * PI / SR;
  p = fmod(micros() * SI, 2 * PI);
  out = (sin(p) * volume + 1) / 2;
  return out;
}
float noise(float volume)
{
  out = random(10000) - 5000 / 10000.0 * volume;
  return out;
}

float saw(float freq, float volume)
{
  float SI = freq * 2 * PI / SR;
  p = fmod(micros() * SI, 2 * PI);
  out = ((p / PI - 1) * volume + 1) / 2;
  return out;
}

float tri(float freq, float volume)
{
  float SI = freq * 2 * PI / SR;
  p = fmod(micros() * SI, 2 * PI);
  float sig = (p / PI < 1 ? p / PI * 2 : 2 - (p / PI - 1) * 2) - 1;
  out = (sig * volume + 1) / 2;
  return out;
}

float square(float freq, float volume)
{
  float SI = freq * 2 * PI / SR;
  p = fmod(micros() * SI, 2 * PI);
  float sig = (p / PI > 1 ? 2 : 0) - 1;
  out = (sig * volume + 1) / 2;
  return out;
}

void vis(int out)
{
  for (int i = 0; i < out / 4; i++)
  {
    Serial.print("_");
  }
  Serial.println();
}

void playSine()
{
  float output = sine(freq, 0.005);

  int out = output * level;

  dac_output_voltage(DAC_CHANNEL_1, out);
}
void playSaw()
{
  float output = saw(freq, 0.005);

  int out = output * level;

  dac_output_voltage(DAC_CHANNEL_1, out);
}
void playTri()
{
  float output = tri(freq, 0.005);

  int out = output * level;

  dac_output_voltage(DAC_CHANNEL_1, out);
}
void playSqr()
{
  float output = square(freq, 0.005);

  int out = output * level;

  dac_output_voltage(DAC_CHANNEL_1, out);
}

void setup()
{
  Serial.begin(115200);
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);

  delay(1000);
  Serial.println("Started up...");
  dac_output_enable(DAC_CHANNEL_1);
}

void loop()
{
  if (presetChanger > presetTime)
  {
    presetChanger = 0;
    presetTime = random(1000000, 5000000);
    if (random(0, 10) > 5)
    {
      preset = 9;
    }
    else
    {
      preset = random(0, 5);
      Serial.print(preset);
    }
  }

  float output = 0;
  if (preset == 0)
  {
    output = sine(freq, sine(1, 1));
  }
  else if (preset == 1)
  {
    output = saw(freq, tri(8, 0.1));
  }
  else if (preset == 2)
  {
    output = (saw(freq, 0.1) + tri(freq * 1.1, 0.1)) / 2;
  }
  else if (preset == 3)
  {
    output = (sine(freq, 0.1) + sine(freq * 2.05, 0.1)) / 2;
  }
  else if (preset == 4)
  {
    output = (sine(sine(freq, 1), 0.1) + sine(freq * 2.05, 0.1)) / 2;
  }
  else
  {
    output = 0;
  }
  int out = output * 255;
  dac_output_voltage(DAC_CHANNEL_1, out / 16);

  mesh.update();
  presetChanger++;
}

void receivedCallback(uint32_t from, String &msg)
{
  freq = msg.toFloat();
  if (freq > 10000)
  {
    freq = freq / 16;
  }
  else if (freq > 1000)
  {
    freq = freq / 4;
  }
  freq = freq * 0.98;
}
