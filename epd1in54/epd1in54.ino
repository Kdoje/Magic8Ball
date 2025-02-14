#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

#define EPD_CS    10
#define EPD_DC    9
#define EPD_RST   8
#define EPD_BUSY  7

// Try different drivers: GxEPD2_154_D67, GxEPD2_154_T8
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
boolean displayed = false;
String strings[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};

void waitWhileBusy() {
  Serial.print("Waiting for BUSY...");
  while (digitalRead(EPD_BUSY) == LOW) {  // Change to HIGH if needed
    delay(100);
  }
  Serial.println("Done.");
}

void setup() {
  Serial.begin(9600);
 // while(!Serial);
  pinMode(EPD_BUSY, INPUT);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  randomSeed(analogRead(0));

  display.init(115200);

  Serial.println("Starting one-shot display");
  display.setRotation(2);  // 2 = 180-degree rotation

  Serial.println("Displaying text...");
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);  // White background

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold24pt7b);
  display.setCursor(10, 50);  // X=10, Y=50
  display.print("I'm \nfucking\n him");

  display.display();  // Only updates once
  // display.powerOff(); // Saves power
}

void loop() {
  if (!displayed) {
  // Nothing needed in loop
    display.fillScreen(GxEPD_WHITE);  // White background

    // display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(10, 50);  // X=10, Y=50
    int randomIndex = random(0, sizeof(strings) / sizeof(strings[0]));
    display.print("A: \n" + strings[randomIndex]);
    display.display();
    displayed = true;
    display.powerOff();
  }
  float x, y, z;

    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
  
      if (x > .5 || y > .5 || z > 1.5) {
        Serial.println("lift detected");
        digitalWrite(LED_BUILTIN, HIGH);
        displayed = false;
      }
  
    }
}
