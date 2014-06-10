#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <RTClib.h>

RTC_DS1307 RTC;

#define PIN 8

// Setting up the buttons
#define lightBtn 2
#define noizeBtn 3

#define speakerPin 11


uint8_t lightReading;           // the current reading from the input pin
uint8_t noizeReading;           // the current reading from the input pin

uint8_t lightState    = LOW;      // the current state of the output pin
uint8_t noizeState    = LOW;      // the current state of the output pin

uint8_t lightPrevious = HIGH; 
uint8_t noizePrevious = HIGH; 

uint32_t lightTime    = 0;         // the last time the output pin was toggled
uint32_t noizeTime    = 0;         // the last time the output pin was toggled

uint32_t lightDebounce = 200;   // the debounce time, increase if the output flickers
uint32_t noizeDebounce = 200;   // the debounce time, increase if the output flickers

// Buttons done

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(12, PIN);

boolean  IsASleep = false;

uint16_t  minsTillWakeup    = 0;      // Minutes till wakeup time
uint16_t  minsPrLight       = 0;      // Minutes pr light
uint8_t   numOfLights       = 1;      // How many lights to light;
uint16_t  timinnnuna        = 0;      // timinn - 6:45 verður 645 og 7:30 verður 730
uint16_t  wakeuptime;                 // Normal wakeup time Mon-Fri
uint16_t  wakeuptimeNrm     = 643;    // Normal wakeup time Mon-Fri
uint16_t  wakeuptimeWnd     = 653;    // Weekend wakeup time Sat & Sun
uint16_t  slokkvadagljos    = 730;
uint16_t  sleeping          = 1845;   // When to hit sleeping mode
uint16_t  hadegiStart       = 1200;
uint16_t  hadegiStop        = 1400;

uint8_t todayIsDay = 0;

uint8_t  mode   = 1, // Current animation effect
offset = 0; // Position of spinny eyes
uint32_t warningColor  = 0xFF8000; // Warning - orange
uint32_t abnormalColor  = 0xFFFF00; // Abnormal - yellow
uint32_t color  = 0x0000FF; // Start Blue
uint32_t prevTime;


uint8_t NoiceFrequency = 50; // Frequency in microseconds
uint32_t soundCounter = 0;
#define aMinute 60000 // 60.000 mills = 1 minute
#define NumLoopsPM (aMinute*100)/NoiceFrequency // Number of times the NoiceFrequency is run pr minute  

uint32_t reg = 0x55aa55aaL;;


DateTime now;

void setup() {
  Serial.begin(57600);
  
  pinMode(lightBtn, INPUT);
  pinMode(noizeBtn, INPUT);
  pinMode(speakerPin, OUTPUT);
  
  soundCounter = NumLoopsPM + 1;  // Make sure we fetch time on first loop
  
  Wire.begin();
  RTC.begin();
  
  pixels.begin();
  prevTime = millis();

  //if (! RTC.isrunning()) {
    //Serial.println("RTC is NOT running!");
  //  showState = 0; // Show a warning light
  //}
}

void loop() {

  if (soundCounter >= NumLoopsPM) {
    // Only get new time once pr minute as pulling new time will mess with the white noise
    soundCounter = 0;
    now = RTC.now();
    timinnnuna = (now.hour()*100)+now.minute();
    todayIsDay = now.dayOfWeek();
      
    if (todayIsDay == 0 || todayIsDay == 6) wakeuptime = wakeuptimeWnd;  // Sun = 0, Sat = 6
    else wakeuptime = wakeuptimeNrm;
      
  } else {
    soundCounter++;
  }
  

  lightReading = digitalRead(lightBtn);
  noizeReading = digitalRead(noizeBtn);
  
  // Light button control
  if (lightReading == HIGH && lightPrevious == LOW && millis() - lightTime > lightDebounce) {
    if (lightState == HIGH) {
      lightState = LOW;
    } else {
      lightState = HIGH;
    }
    lightTime = millis();    
  }
  lightPrevious = lightReading;

  // Noize button control
  if (noizeReading == HIGH && noizePrevious == LOW && millis() - noizeTime > noizeDebounce) {
    if (noizeState == HIGH) {
      noizeState = LOW;
    } else {
      noizeState = HIGH;
      // Reading
    }
    noizeTime = millis();    
  }
  noizePrevious = lightReading;
  
  uint8_t  i;
  uint32_t t;

  if (lightState == HIGH) {
    readingMode();
  } else if (!timinnnuna) {
    // Something is not right, warning lights!
    chaseTail(warningColor, 100);
    
  } else if (timinnnuna >= wakeuptime && timinnnuna <= slokkvadagljos) {
    // Sunrise
    IsASleep = false;
    sunriseMode();

  } else if (timinnnuna >= sleeping || timinnnuna < wakeuptime){
    IsASleep = true;

    minsTillWakeup = 690;
    if (timinnnuna == sleeping)  minsPrLight = minsTillWakeup / 12;  //
    minsTillWakeup = minutesTillWakeup(now);
    numOfLights = numberOfLights(minsTillWakeup, minsPrLight);

    Serial.println(numOfLights);
    Serial.println(minsTillWakeup);
    Serial.println("---");

    sleepingMode(numOfLights);

  }  else if (timinnnuna >= hadegiStart && timinnnuna < hadegiStop) {
    // Noon - just noize, no light
   IsASleep = true;
   offMode();

  } else { 
    // Nothing going on, everything turned off
    IsASleep = false;
    offMode();
  }
  

  if (noizeState == HIGH) generateNoise();
  else delayMicroseconds(NoiceFrequency);

}

void chaseTail(uint32_t useColor, uint8_t useDelay){
  pixels.setBrightness(20);
  uint8_t  i;
    for(i=0; i<12; i++) {
      uint32_t c = 0;
      if(((offset + i) & 3) < 2) c = warningColor; // 4 pixels on...
      pixels.setPixelColor(i, c); // First eye
    }
    pixels.show();
    offset++;
    delay(useDelay);
  
}

void sleepingMode(uint8_t numOfLights) {



  pixels.setBrightness(5);
  for (int i = 0; i<numOfLights;i++) {
    pixels.setPixelColor(i, 0x0000FF);
  }
  for (int j = numOfLights; j<12; j++) {
    pixels.setPixelColor(j, 0x000000);
  }
  pixels.show();
}

void sunriseMode() {
  pixels.setBrightness(50);
  for (int i = 0; i<12;i++) {
    pixels.setPixelColor(i, 0xFF8000);
  }
  pixels.show();
}

void readingMode() {
  // All set to full brightness and white
  pixels.setBrightness(100);
  for (int i = 0; i<12;i++) {
    pixels.setPixelColor(i, 0xFFFFFF);
  }
  pixels.show();
}

void offMode() {
  // All set to full brightness and white
  pixels.setBrightness(0);
  for (int i = 0; i<12;i++) {
    pixels.setPixelColor(i, 0x000000);
  }
  pixels.show();
}

void printDate(DateTime now) {
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

}

void generateNoise(){
  unsigned long int newr;
  unsigned char lobit;
  unsigned char b31, b29, b25, b24;
  b31 = (reg & (1L << 31)) >> 31;
  b29 = (reg & (1L << 29)) >> 29;
  b25 = (reg & (1L << 25)) >> 25;
  b24 = (reg & (1L << 24)) >> 24;
  lobit = b31 ^ b29 ^ b25 ^ b24;
  newr = (reg << 1) | lobit;
  reg = newr;
  digitalWrite (speakerPin, reg & 1);
  delayMicroseconds(NoiceFrequency); // Changing this value changes the frequency of the sound. 
  //Serial.println(soundCounter);
} 

uint16_t minutesTillWakeup(DateTime TimeNow) {
  uint16_t hoursFromMidnight = (wakeuptime/100);
  uint16_t fromMidnight = (hoursFromMidnight*60)+(wakeuptime-(hoursFromMidnight*100));
  
  uint16_t tillMidnight = ((23-TimeNow.hour())*60)+(60-TimeNow.minute());
  
  return hoursFromMidnight + tillMidnight;  
}

uint16_t numberOfLights(uint16_t minTillWakeup, uint16_t minPrLight) {
  uint16_t numLights = round(minTillWakeup/minPrLight);
  return numLights;
}

 

