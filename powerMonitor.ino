// Power Monitor
// Coded by Ryan Jin for the ES1050 Winter Project "Power" for the scholastic term 2024-2025.
// Utilizing the GxEPD2 library Jean-Marc Zingg (the G.O.A.T.)
// This program is NOT a final product, nor is it to be assumed to be so.
// Warnings and limitations will be discussed:
// The voltage readings calculated by the code are taken from a voltage divider that does NOT denoise using capacitors. This means there is an linear function in terms of
// error and voltage. (i.e. margin of error increases when voltage increases, albeit not too much)

// While this project does expect measurements for long periods of time, I have taken the liberty to treat this program as it is- a prototype test program. That is to say, the time keeping
// function is using the built in millis(), which WILL overflow back to zero in ~50 days after the code begins to run on the microcontroller. This means that the recommended run time for
// this program is only the amount of time we have run it during our showcase on April 1st, which I estimate to be ~1 hr. lol.

// I have not figured out how to save data in a 2D array yet. You can actually see the method commented out, as well as the 2D array I inteded to use as the method of saving data commented
// out as well.

// In terms of power consumption, the current build is already pretty energy efficient. However, it can be made even more efficient, by decreasing the CPU frequency to 80MHz, entering the 
// ESP32's deep sleep function, and using interrupts rather than constantly checking for button inputs. Disabling unused functions, such as WiFi and Bluetooth (when not being used) and extra
// GPIO pins.

// Probably could save a shit ton of storage by removing some of these since a few of these are definitely redundant.
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"
// Record of the GPIO pins for SPI communication
#define CS_PIN     5    // Chip Select Pin
#define SS_PIN     5    // Slave Select Pin (these two are technically the same thing)
#define RST_PIN    16   // Reset Pin
#define DC_PIN     17   // Data/Command Pin
#define BUSY_PIN   4    // Busy Pin
#define MISO_PIN   19   // Master In Slave Out Pin (not needed, technically redundant but code might need it, so we reserve pin 19 for it.) (alternative could also be to set this to -1)
#define MOSI_PIN   23   // Master Out Slave In Pin (Data out pin basically. TL;DR: Master is CPU, Slave is device.)
#define SCK_PIN    18   // Serial Clock Pin
// Define button pins and other button related settings
#define UBUTTON_PIN 33
#define RBUTTON_PIN 12
#define DBUTTON_PIN 14
#define LBUTTON_PIN 27
#define debounceTime 50
// Definitions for B&W
#define UNCOLORED GxEPD_WHITE
#define COLORED GxEPD_BLACK
// definitions for the tick widths on the axes of the graph
#define xTickWidth 25
#define yTickWidth 36
// voltage conditions
#define MAX_VOLTAGE 12.0
#define ADC_PIN 2
#define DIVIDER_RATIO (71.831 / 1071.831)

// some variables that need to be declared at a global scope
unsigned long fiveMinuteCounter = 0;
unsigned long lastFiveMinute = 0;
int page;
int lastXPos;
float lastYPos;

// 

void setup() {
  Serial.begin(600);
  display.init(115200, true, 2, false);  // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  pinMode(UBUTTON_PIN, INPUT_PULLDOWN);
  pinMode(RBUTTON_PIN, INPUT_PULLDOWN);
  pinMode(DBUTTON_PIN, INPUT_PULLDOWN);
  pinMode(LBUTTON_PIN, INPUT_PULLDOWN);
  drawGraphPage();
  drawDataPoint(voltageRead(), fiveMinuteCounter);
}

void drawGraphAxes() {
  display.setRotation(1);
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(UNCOLORED);

  const char graphTitle[] = "Volts vs. Time";
  const char maxVoltage[] = "12V";
  const char maxTime[] = "30 min";
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(COLORED);
  int16_t textX, textY;
  uint16_t textW, textH;

  display.getTextBounds(graphTitle, 0, 0, &textX, &textY, &textW, &textH);
  uint16_t x = ((display.width() - textW) / 2) - textX;
  uint16_t y = 150 + textH * 2;
  do {
    display.setCursor(x, y);
    display.print(graphTitle);
  } while (display.nextPage());

  do {
    // Draws the axes
    display.drawLine(20, 150, 170, 150, COLORED);  // X-Axis
    display.drawLine(20, 150, 20, 6, COLORED);     // Y-Axis

    // Draws ticks on the X-Axis
    for (int i = 45; i <= 170; i += xTickWidth) {
      display.drawLine(i, 148, i, 152, COLORED);  // Small tick marks, 5 minutes per tick mark
    }

    // Draws ticks on the Y-Axis
    for (int i = 114; i >= 6; i -= yTickWidth) {
      display.drawLine(18, i, 22, i, COLORED);  // Small tick marks, 4V per tick
    }
  } while (display.nextPage());
}

//void drawSavedData(double** data) {                               i have no fucking clue how to code with arrays in c++
//  for (int i = 0; i < 1; i++) {
//    for (int j = 0; j < data[i]; j++) {
//      int xPosition = 20 + fiveMinuteCounter * xTickWidth;
//      int yPosition = 6 + voltage * 12;
//      do {
//        display.fillCircle(xPosition, yPosition, 2, COLORED);
//      } while (display.nextPage());
//    }
//  }
//}

void drawDataPoint(float voltage, int fiveMinuteCounter) {
  Serial.println("Me too!!!!");
  int xPosition = 20 + fiveMinuteCounter * xTickWidth;
  float yPosition = 150 - (voltage * 12);

  do {
    display.fillCircle(xPosition, yPosition, 2, COLORED);
    if (fiveMinuteCounter != 0) {
      display.drawLine(lastXPos, lastYPos, xPosition, yPosition, COLORED);
    }
    lastXPos = xPosition;
    lastYPos = yPosition;
  } while (display.nextPage());
}

void drawGraphPage() {
  page = 1;
  display.fillScreen(UNCOLORED);
  drawGraphAxes();
  drawButtonInfo();
}

float voltageRead() {
  int rawReading = analogRead(ADC_PIN);
  float measuredVoltage = (rawReading / 4095.0) * 3.3;  // Convert reading to ESP32 voltage value (I can explain this in person)
  float voltage = measuredVoltage / DIVIDER_RATIO;      // convert to "real" voltage
  Serial.println(voltage);
  return voltage;
}

bool upButtonPressCheck() {
  static unsigned long upLastPress = 0;  // could save more memory by having the lastPress not be specific to each button... but I can just say that it's for future simultaneous button presses

  if (digitalRead(UBUTTON_PIN) == HIGH) {
    if (millis() - upLastPress > debounceTime) {
      upLastPress = millis();
      do { display.fillCircle(100 + 85, 94 + 85, 3, UNCOLORED); } while (display.nextPage());
      return true;
    }
  }
  return false;
}

bool rightButtonPressCheck() {
  static unsigned long rightLastPress = 0;

  if (digitalRead(RBUTTON_PIN) == HIGH) {
    if (millis() - rightLastPress > debounceTime) {
      rightLastPress = millis();
      do { display.fillCircle(106 + 85, 100 + 85, 3, UNCOLORED); } while (display.nextPage());
      return true;
    }
  }
  return false;
}

bool downButtonPressCheck() {
  static unsigned long downLastPress = 0;

  if (digitalRead(DBUTTON_PIN) == HIGH) {
    if (millis() - downLastPress > debounceTime) {
      downLastPress = millis();
      do { display.fillCircle(100 + 85, 106 + 85, 3, UNCOLORED); } while (display.nextPage());
      return true;
    }
  }
  return false;
}

bool leftButtonPressCheck() {
  static unsigned long leftLastPress = 0;

  if (digitalRead(LBUTTON_PIN) == HIGH) {
    if (millis() - leftLastPress > debounceTime) {
      leftLastPress = millis();
      do { display.fillCircle(94 + 85, 100 + 85, 3, UNCOLORED); } while (display.nextPage());
      return true;
    }
  }
  return false;
}

void drawButtonInfo() {
  do {
    display.fillCircle(100 + 85, 106 + 85, 4, COLORED);
    display.fillCircle(106 + 85, 100 + 85, 4, COLORED);
    display.fillCircle(100 + 85, 94 + 85, 4, COLORED);
    display.fillCircle(94 + 85, 100 + 85, 4, COLORED);
  } while (display.nextPage());
}

void pageCheck() {
}

void loop() {
  fiveMinuteCounter = millis() / (1000UL * 60 * 5);
  Serial.print("Counter: ");
  Serial.println(fiveMinuteCounter);
  Serial.print("Last count: ");
  Serial.println(lastFiveMinute);
  if (fiveMinuteCounter > lastFiveMinute) {
    Serial.println("I'M WORKING!!!");
    drawDataPoint(voltageRead(), fiveMinuteCounter);
    lastFiveMinute = fiveMinuteCounter;
  }
  if (upButtonPressCheck()) {
    delay(50);
    drawButtonInfo();
  } else if (rightButtonPressCheck()) {
    delay(50);
    drawButtonInfo();
  } else if (downButtonPressCheck()) {
    delay(50);
    drawButtonInfo();
  } else if (leftButtonPressCheck()) {
    delay(50);
    drawButtonInfo();
  }
}
