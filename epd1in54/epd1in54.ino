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
BLEService textService("180C");  // Custom service UUID
BLECharacteristic textCharacteristic("2A56", BLEWrite | BLEWriteWithoutResponse, 600); // Max 50 chars


// Try different drivers: GxEPD2_154_D67, GxEPD2_154_T8
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
boolean displayed = false;
String strings[20];
int currentItemCount = 20;

void waitWhileBusy() {
  Serial.print("Waiting for BUSY...");
  while (digitalRead(EPD_BUSY) == LOW) {  // Change to HIGH if needed
    delay(100);
  }
  Serial.println("Done.");
}

void setup() {
  Serial.begin(9600);
  strings[0] = "Apple";
  strings[1] = "Banana";

  for (int i = 2; i < sizeof(strings) / sizeof(strings[0]); i++) {
    strings[i] = "Uninit" + String(i);
  }

 // while(!Serial);
  pinMode(EPD_BUSY, INPUT);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("Failed to start BLE!");
    while (1);
  }

  BLE.setLocalName("Nano33IoT");  // Bluetooth name
  BLE.setAdvertisedService(textService);
  textService.addCharacteristic(textCharacteristic);
  BLE.addService(textService);

  textCharacteristic.writeValue(""); // Initialize empty
  BLE.advertise();  // Start advertising

  Serial.println("Waiting for BLE connections...");

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

void drawCenterString(String buf) {
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &textWidth, &textHeight); //calc width of new string
    display.setCursor(0, (200-textHeight)/2+30);
   //display.setCursor(50, 25);
    display.print(buf);
}

void parseEntries(String input) {
    const int MAX_ITEMS = sizeof(strings) / sizeof(strings[0]);
    const char DELIMITER = ',';
    int count = 0;
    int startIndex = 0;
    currentItemCount = 0;
    int endIndex = input.indexOf(DELIMITER);

    for (int i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
      strings[i] = "";
    }

    while (endIndex != -1 && count < MAX_ITEMS) {
          strings[count++] = input.substring(startIndex, endIndex);  // Extract substring
          startIndex = endIndex + 1;  // Move start past the delimiter
          endIndex = input.indexOf(DELIMITER, startIndex);  // Find next delimiter
    }

    // Add the last part (or full string if no delimiter found)
    if (startIndex < input.length() && count < MAX_ITEMS) {
        strings[count++] = input.substring(startIndex);
    }
    Serial.println("Strings parsed are:");
    for (int i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
      Serial.print(strings[i]);
      Serial.print(";;");
    }
    Serial.println();
    currentItemCount = count;
}

void loop() {
  BLEDevice central = BLE.central();

  if (central.connected()) {  // Stay connected
    if (textCharacteristic.written()) {  // If data is received
      char receivedText[600] = {0}; // Buffer for incoming text
      textCharacteristic.readValue(receivedText, sizeof(receivedText));
      parseEntries(receivedText);
      Serial.println(receivedText);
    }
  }
  if (!displayed) {
  // Nothing needed in loop
    display.fillScreen(GxEPD_WHITE);  // White background

    // display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeMonoBold24pt7b);
    //display.setCursor(10, 50);  // X=10, Y=50
    int randomIndex = random(0, currentItemCount);
    // display.print(strings[0]);
    drawCenterString(strings[randomIndex]);
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
