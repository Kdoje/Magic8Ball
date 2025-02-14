#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#define EPD_CS    10
#define EPD_DC    9
#define EPD_RST   8
#define EPD_BUSY  7

// Try different drivers: GxEPD2_154_D67, GxEPD2_154_T8
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

void waitWhileBusy() {
  Serial.print("Waiting for BUSY...");
  while (digitalRead(EPD_BUSY) == LOW) {  // Change to HIGH if needed
    delay(100);
  }
  Serial.println("Done.");
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(EPD_BUSY, INPUT);
  display.init(115200);

  Serial.println("Forcing full refresh...");
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);  // Change font as needed
    display.setCursor(10, 50);  // X=10, Y=50
    display.print("Hello, E-Ink!");
  } while (display.nextPage());

  display.powerOff();
}

void loop() {
  // Nothing needed in loop
}
