#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>


#define EPD_CS    10
#define EPD_DC    9
#define EPD_RST   8
#define EPD_BUSY  7
// Use pin 13 on the arduino for the CLOCK signal and pin 11 for the COPI (controller out, peripheral in).
// connect these to the corresponding pins on the display (CLK & SDI respectively)

BLEService textService("180C");  // Custom service UUID
BLECharacteristic textCharacteristic("2A56", BLEWrite | BLEWriteWithoutResponse, 3000);


// Try different drivers: GxEPD2_154_D67, GxEPD2_154_T8
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
boolean displayed = false;
String phrases[20] = {
    "You can't get mad at a guy for something he did in his early 30s",
    "america has an opioid crisis and you are leveraging this for your antics",
    "I'm f***ing embarrassed to be an indie game dev",
    "another day volunteering at the betsy ross museum. everyone keeps asking me if they can fuck the flag.",
    "vaulting my big ass over the tables at the vancouver homeowners association",
    "What's the last 4 digits of your social security number?",
    "I don't like being on planet earth anymore I rather be up in outer space, in my opinion.",
    "I was into the glowjng people and the wiggly enemies and weird sand and then the game said glue king. I'm game ended",
    "anybody else love they self? like I really f*** with me heavy",
    "Peach had to of thrown at least one chair.... right?",
    "Lmfao scott w the idea that society should function and you all should be kind to one another is crazyyyyy",
    "It’s just a bad janny, not a bad week",
    "Look yeah I hate yoshi",
    "Developers are indispensable for this service. But this service must be designed w/o including them",
    "I dont like taking a sip of house medicine and the first shot is a small splash in the mouth.",
    "oh, youre here from tik tok?  (chuckle's lightly ) Is that like reddit for babies",
    "i wish mr bean could talk so he could say some shit like \"OK that was pretty messed up.\"",
    "did it hurt when you fell from heaven and kissed my stupid ass you mother fucker",
    "i shou;ld be allowed on the roof of any building",
    "i dont care what Yankee Doodle did in town. His toxic fanbase tells me everything."
};
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

  BLE.setLocalName("The8Ball");  // Bluetooth name
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
  display.fillScreen(GxEPD_BLACK);  // White background

  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold24pt7b);
  display.setCursor(10, 50);  // X=10, Y=50
  display.print("8-Ball\nloading\n...");

  display.display();  // Only updates once
  // display.powerOff(); // Saves power
}

String formatString(String input, int maxLineLength = 14) {
    String output = "";
    int lineLength = 0;

    for (int i = 0; i < input.length(); i++) {
        if (input[i] == ' ') {
            // Find next word length
            int nextWordLength = 0;
            int j = i + 1;
            while (j < input.length() && input[j] != ' ') {
                nextWordLength++;
                j++;
            }

            // If adding the next word exceeds the max length, insert a newline
            if (lineLength + nextWordLength + 1 > maxLineLength) {
                output += '\n';
                lineLength = 0;
            } else {
                output += ' ';
                lineLength++;
            }
        } else {
            output += input[i];
            lineLength++;
        }
    }

    return output;
}

void drawCenterString(String buf) {
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &textWidth, &textHeight); //calc width of new string
    display.setCursor(0, (200-textHeight)/2+5);
   //display.setCursor(50, 25);
    display.print(formatString(buf));
}

void parseEntries(String input) {
    const int MAX_ITEMS = sizeof(phrases) / sizeof(phrases[0]);
    const char DELIMITER = '|';
    int count = 0;
    int startIndex = 0;
    currentItemCount = 0;
    int endIndex = input.indexOf(DELIMITER);

    for (int i = 0; i < sizeof(phrases) / sizeof(phrases[0]); i++) {
      phrases[i] = "";
    }

    while (endIndex != -1 && count < MAX_ITEMS) {
          phrases[count++] = input.substring(startIndex, endIndex);  // Extract substring
          startIndex = endIndex + 1;  // Move start past the delimiter
          endIndex = input.indexOf(DELIMITER, startIndex);  // Find next delimiter
    }

    // Add the last part (or full string if no delimiter found)
    if (startIndex < input.length() && count < MAX_ITEMS) {
        phrases[count++] = input.substring(startIndex);
    }
    Serial.println("phrases parsed are:");
    for (int i = 0; i < sizeof(phrases) / sizeof(phrases[0]); i++) {
      Serial.print(phrases[i]);
      Serial.print(";;");
    }
    Serial.println();
    currentItemCount = count;
}

void loop() {
  BLEDevice central = BLE.central();

  if (central.connected()) {  // Stay connected
    if (textCharacteristic.written()) {  // If data is received
      Serial.println("Received data");
      char receivedText[3000] = {0}; // Buffer for incoming text
      textCharacteristic.readValue(receivedText, sizeof(receivedText));
      parseEntries(receivedText);
      Serial.println(receivedText);
    }
  }
  if (!displayed) {
  // Nothing needed in loop
    display.fillScreen(GxEPD_BLACK);  // White background

    // display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeMonoBold12pt7b);
    //display.setCursor(10, 50);  // X=10, Y=50
    int randomIndex = random(0, currentItemCount);
    // display.print(phrases[0]);
    drawCenterString(phrases[randomIndex]);
    display.display();
    displayed = true;
    display.powerOff();
  }
  float x, y, z;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    if (x > 1.5 || y > 1.5 || z > 1.5) {
      Serial.println("lift detected");
      digitalWrite(LED_BUILTIN, HIGH);
      displayed = false;
    }

  }
}
