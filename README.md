
### Important details:
1. Consult phone pictures for wiring, 
2. The system uses 3.3V for power -- we can use the battery to power this as a result

### To build
* Open `epd1in54.ino` in the arduino IDE to properly deploy the code
* build and deploy the `epd1in54.ino` to the IoT nano 33 with Inland 1.54" eink display connected to SPI to get the different phrases generating
* Send data to the exposed Bluetooth endpoint using LightBlue
    * Go to Peripherals
    * Connect to `The8Ball`
    * Write to the Service ending in `...9b34fb`
    * Write a UTF-8 string value using `|` to separate entries. For example
    `Open your third eye|consider purchasing dominos|Go to Jail. Do not pass go, Do not collect $200`
    * Then shake the ball to observe one of your entries appear

### Limitations
* You can have 20 phrases with about a 480 character limit in the message to set them. You can probably expand this with a more complex packet protocol, but it works good enough for now

### FAQ
1. Is there a wiring diagram
    1. No, I haven't had time to put one together. If you want one, please open an issue (or +1 it) and I'll supply one if there's a need