# Smart home project using NodeMCU.
Links provided below for purchased parts

## Requirements
There are probably cheaper options, if found please link here. 
* NodeMCU 
    * https://www.amazon.com/gp/product/B01ISYTADW/ref=oh_aui_detailpage_o06_s01?ie=UTF8&psc=1
* High temperature (brightness) White LED
    * https://www.amazon.com/gp/product/B076RHLF7Y/ref=oh_aui_detailpage_o07_s00?ie=UTF8&psc=1
* High temperature RGB individual addressable LED (WS2812B)
    * https://www.amazon.com/gp/product/B01LSF4Q00/ref=oh_aui_detailpage_o05_s00?ie=UTF8&psc=1
* 12V power supply 5A 
    * https://www.amazon.com/gp/product/B01461MOGQ/ref=oh_aui_detailpage_o05_s00?ie=UTF8&psc=1
* Variable buck converter 5A rating 
    * https://www.amazon.com/gp/product/B07BF7RDXT/ref=oh_aui_detailpage_o04_s00?ie=UTF8&psc=1
* RFP30N06LE Logic level Mosfet 
    * https://www.amazon.com/gp/product/B07CTF1JVD/ref=oh_aui_detailpage_o05_s00?ie=UTF8&psc=1
* spare resistors (two 330 and one 1M Ω)
    * https://www.amazon.com/gp/product/B016NXK6QK/ref=oh_aui_detailpage_o06_s02?ie=UTF8&psc=1

## Software requirements 
 * Blynk mobile app 
 * Arduino IDE
 * Arduino Neopixel library
 * ESP8266 board Arduino IDE install 


## Background

For those who live in an area where sunlight is a luxury, this project is a great way to replenish the need for natural light. This smart sunlight alarm aims to fix the issue by providing an open source software for the components listed above. 

Below is a link to a video for the finished product. 

Special thanks to lukecyca for the influence and base code. https://github.com/lukecyca/partycat-sunrise-alarm-clock. 

## Basic Instructions 

1. Install the necessary list of software above, for Blynk use a mobile app. This app acts as an interface for NodeMcu. Once set up, the app can control the device from outside the network by sending the request to the Blynk cloud server (by default).  
1. Wire components accordingly. As a note there a few warnings that should be followed when constructing the circuit. 
    * When connecting the pin from NodeMcu and the data line of the RGB LED, connect a resistor (I used a 330Ω resistor). This is to limit the current going to the IC WS2812B of the LED. 
    * When conecting the white led strip, add a high resistance resisitor (1MΩ) to filter out low voltage that the mosfet can read. Also connect a small resistor to the data pin of the NodeMcu to the Gate of the Mosfet. 
    * Keep the 5A power supply **Unplugged** while uploading script. USB is enough to power a few LEDs if need be. 
    * If unsure connect a few Leds at a time to the Vin or 3.3 V of the NodeMcu to test the logic of the script. 
1. In the Blynk app add a switch button and connect to virutal pin 2 (V2), slider to V1 and Time Input to V3. 
1. Connect the NodeMcu to the computer using a USB cable and Upload the script.
    * Note: there are a few files not used in this project, ex. ntp.h, server.h, etc. 
1. Once uploaded open the Arduino Serial Output and select 115200 Baud and make sure the NodeMcu is connected to the Blynk server. 
1. Change some of the time setting to make sure the values are set properly. 
1. Disconnect from the USB and connect the NodeMcu to the circuit with the rest of the circuit. 
1. Connect the Vin of the NodeMcu to 5V output of the variable buck converter
1. Enjoy the light. 

## Optional 
* Alexa compatibility 
* Google Home compatibility
* Siri compatibility 
* Blynk UI improvements 
* Other improvements 

