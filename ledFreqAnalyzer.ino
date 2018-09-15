#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define LED_NUM 16 // number of LEDs per strip
#define STRIP_NUM 16 // number of strips used

AudioInputI2S            i2s1;           //xy=200,69
AudioOutputI2S           i2s2;           //xy=580,158
AudioMixer4              mixer1;         //xy=343,200
AudioAnalyzeFFT256       fft256_1;       //xy=581,234
AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, 0, i2s2, 0);
AudioConnection          patchCord4(mixer1, 0, i2s2, 1);
AudioConnection          patchCord5(mixer1, fft256_1);
AudioControlSGTL5000     sgtl5000_1;     //xy=189,308

Adafruit_NeoPixel pixelStrip = Adafruit_NeoPixel(256, 6, NEO_GRB + NEO_KHZ800);

const int myInput = AUDIO_INPUT_LINEIN; // for i2s input
int currentMax[STRIP_NUM];
int millisDelay[STRIP_NUM];
int colorPin = 14;
float colorVal = 0;
int gainPin = 15;
double gainVal = 0;

void setup() {
  // initialize led strips to 'off'
  pixelStrip.begin();
  pixelStrip.show();
  // initialize max indicator and delay variables
  for (int i = 0; i < STRIP_NUM; i++) {
    currentMax[i] = 0;
    millisDelay[i] = 0;
  }

  // initialize audion board input, volume, and gain
  Serial.begin(9600);
  AudioMemory(12); // for i2s input
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5); 
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);

  delay(1000);
}

void loop() {
  // change color using potentiometer
  colorVal = analogRead(colorPin);
  colorVal = colorVal / 900;
  //Serial.println(colorVal);

  // change LED levels using potentiometer
  gainVal = analogRead(gainPin);
  gainVal = gainVal / 50; // gain is between 1 and ~20
  //Serial.println(gainVal);
  
  if(fft256_1.available()) { // if fft data is available
    updateStrip(gainVal); // call updateStrip function
  }
}

void updateStrip(int GAIN) {
  // color constants
  uint32_t RED = pixelStrip.Color(100, 0, 0);
  uint32_t BLUE = pixelStrip.Color(0, 0, 100);
  uint32_t BLK = pixelStrip.Color(0, 0, 0);
  
  int num_lit[STRIP_NUM];
  int evens = 0; // LED offset for even strips
  int odds_top = 0; // top LED offset for odd strips
  int odds = 31; // LED offset for odd strips

  // determines number of LEDs to turn on and max indicator for each strip
  for (int i = 0; i < STRIP_NUM; i++) {
    double index = i + 2;
    num_lit[i] = LED_NUM * fft256_1.read(i) * (GAIN * log(index));
    Serial.print(log(index));
    Serial.print(" ");
    Serial.println(GAIN * log(index));

    if (num_lit[i] > LED_NUM) {
      num_lit[i] = LED_NUM;
    }
    if (currentMax[i] < num_lit[i]) {
      currentMax[i] = num_lit[i];
    } 
  }

  // handles even numbered strips starting from 0
  for (uint16_t j = 0; j < STRIP_NUM; j += 2) {
    for (uint16_t i = evens; i < LED_NUM + evens; i++) {
      if (i < (num_lit[j] - 1) + evens) { // all but top lit LED are blue
        pixelStrip.setPixelColor(i, BLUE);
      } else if (i < num_lit[j] + evens) { // handles top LED
        if (currentMax[j] < num_lit[j]) {
          pixelStrip.setPixelColor(i, RED); // red if new max
          millisDelay[j] = millis();
        } else if (currentMax[j] > num_lit[j] && (millis() - millisDelay[j]) > 1000) {
          pixelStrip.setPixelColor(i, RED); // red if delay > 1000
          currentMax[j] = num_lit[j];
          millisDelay[j] = millis();
        } else { // if delay has not passed or no new max
          pixelStrip.setPixelColor(i, BLUE); // last LED is blue
          pixelStrip.setPixelColor(currentMax[j] + evens, RED); // max indicator is red
        }
      } else if (i != currentMax[j] + evens) { // turn all other LEDs off
        pixelStrip.setPixelColor(i, BLK);
      }
    }
    evens += 32; // increase offset
  }

  // handles odd numbered strips starting from 1
  for (uint32_t j = 1; j < STRIP_NUM; j +=2) {
    for (uint16_t i = odds; i > (LED_NUM - 1) + odds_top; i--) {
      if (i > odds - (num_lit[j] - 1)) {
        pixelStrip.setPixelColor(i, BLUE);
      } else if (i > odds - num_lit[j]) {
        if (currentMax[j] < num_lit[j]) {
          pixelStrip.setPixelColor(i, RED);
          millisDelay[j] = millis();
        } else if (currentMax[j] > num_lit[j] && (millis() - millisDelay[j]) > 1000) {
          pixelStrip.setPixelColor(i, RED);
          currentMax[j] = num_lit[j];
          millisDelay[j] = millis();
        } else {
          pixelStrip.setPixelColor(i, BLUE);
          pixelStrip.setPixelColor(odds - currentMax[j], RED);
        }
      } else if (i != odds - currentMax[j]) {
        pixelStrip.setPixelColor(i, BLK);
      }
    }
    odds += 32;
    odds_top += 32;
  }

  pixelStrip.show(); // display LEDs
}
