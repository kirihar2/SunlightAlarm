
#define BLYNK_PRINT Serial
#define DEBUG (0) // Disable degub = 0, enable = 1

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

// Interval in ms to resync NTP
#define ntpSyncInterval (10000)
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = -6;    // Central European Time

unsigned long lastNTPRequest = 0;
unsigned long lastNTPSync = 0;

// Interval in ms to update animation
#define animationInterval (10)
unsigned long lastAnimationUpdate = 0;

// Value of millis() when an animation starts.
// (used as basis for animation timeline)
unsigned long animationStartTime = 0;

boolean turnOffCompletely = false;

// Which animation is presently playing (if any)
#define ANIM_STATE_OFF (0)
#define ANIM_STATE_SUNRISE (1)
#define ANIM_STATE_FADEOUT (2)
#define ANIM_STATE_ON (3)
int animationState = ANIM_STATE_OFF;

// WS2812 layout parameters
#define NUM_LEDS 28
#define LEDS_PER_ROW 14


#define RED_LED_PIN D1
#define WHITE_PIN D2
#define PWM_RANGE_FULL (1024)
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, RED_LED_PIN, NEO_GRB + NEO_KHZ800);


BlynkTimer timer;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "f926197c1ae94bc28b2a8a71d83689fb";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "NETGEAR62";
char pass[] = "gracefulroad125";

#define ledPin D1

void fade() {
  int currentValue = analogRead(ledPin);
  Serial.print("value ");
  Serial.println(currentValue);
  for (int ind = 0; ind <= 1023; ind++) {
    analogWrite(ledPin, ind);
    delay(10);
  }
}
#define SETTINGS_VERSION "s4d3"
struct Settings {
  // Time in seconds after midnight to begin sunrise
  long startTimeInSecs;

  // Time in seconds after midnight to turn off the light
  long stopTimeInSecs;

  int days[7];
  // How quickly to advance the sunrise frames (default 1)
  int fps;

  // Timezone offset
  int timeZone;

  int timeZoneOffset;

} settings = {
  65300,
  78000,
  {0, 0, 0, 0, 0, 0, 0},
  1,
  -7,
  -21600,
};

#include "C:\Users\juran\Documents\Arduino\NodeMCU\Sunlight\NodeMCU2\libdcc\animation.h"

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void sendNTPpacket(IPAddress &address);
void printDigits(int digits);
void setColour(uint8_t r, uint8_t g, uint8_t b);
boolean shouldTurnOnToday();
void saveSettings() {
  EEPROM.put(0, settings);
  EEPROM.put(sizeof(settings), SETTINGS_VERSION);
  EEPROM.commit();
}

void loadSettings() {
  EEPROM.begin(sizeof(settings) + 5);

  char v[5];
  EEPROM.get(sizeof(settings), v);
  if (String(v) == SETTINGS_VERSION) {
    EEPROM.get(0, settings);
    Serial.println("Loaded settings from EEPROM v" + String(v));
  } else {
    Serial.println("Failed to load settings from EEPROM. Using defaults.");
  }
}

void saveTimeSetting(TimeInputParam t) {

  if (t.hasStartTime())
  {
    settings.startTimeInSecs = t.getStartHour() * 3600 + t.getStartMinute() * 60 + t.getStartSecond();
  }
  else if (t.isStartSunrise())
  {
  }
  else if (t.isStartSunset())
  {
  }


  // Process stop time

  if (t.hasStopTime())
  {
    settings.stopTimeInSecs = t.getStopHour() * 3600 + t.getStopMinute() * 60 + t.getStopSecond();

  }

  // Process timezone
  // Timezone is already added to start/stop time

  // Get timezone offset (in seconds)
  settings.timeZoneOffset = t.getTZ_Offset();
  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)

  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      settings.days[i - 1] = i;
    }
    else {
      settings.days[i - 1] = 0;
    }
  }
  saveSettings();
}
void printSettings() {
  Serial.print("Start time : ");
  Serial.println(settings.startTimeInSecs);
   Serial.print("Stop time : ");
  Serial.println(settings.stopTimeInSecs);
  Serial.print("Time Zone Offset : ");
  Serial.println(settings.timeZoneOffset);
  Serial.print("Days: ");
  for (int i = 1; i < 7; i++) {
    Serial.print(settings.days[i - 1]);
    Serial.print(",");
  }
  Serial.println(settings.days[6]);
}
BLYNK_WRITE(V3) {
  TimeInputParam t(param);
  loadSettings();
  // Process start time
  saveTimeSetting(t);
  printSettings();
  setColour(0, 0, 0);

}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  analogWrite(WHITE_PIN, pinValue);
  Serial.println(pinValue);
  //setColour(pinValue%255,0,0);
  
  // process received value
}
BLYNK_WRITE(V2)
{
  int i = param.asInt(); // assigning incoming value from pin V1 to a variable
  setColour(0, 0, 0);
  analogWrite(WHITE_PIN, 0);
  if(i==0){
   animationState =  ANIM_STATE_OFF;
   turnOffCompletely = true;
   } else{
    
      animationState = ANIM_STATE_SUNRISE;
      animationStartTime = millis();
      turnOffCompletely = false;
    }
  // process received value
}
void flash(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(RED_LED_PIN, HIGH);
    delay(50);
    digitalWrite(RED_LED_PIN, LOW);
    delay(100);
  }
}
void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}
int currentSeconds(){
  return (hour()*60+minute())*60+second();  
}
void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
//  Serial.print(ntpServerName);
//  Serial.print(": ");
//  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + settings.timeZoneOffset;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
void setColour(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(r,g,b));

  }
  pixels.show();
  //ledstrip.show(pixels);
}

boolean shouldTurnOnToday(){
  int currentDayOfWeek = weekday();
  if (currentDayOfWeek == 1){
    currentDayOfWeek = 6;  
  }
  return settings.days[currentDayOfWeek]>0;  
}
void setup()
{
  // Debug console
  Serial.begin(115200);
  //pinMode(ledPin, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  //analogWrite(ledPin, 0);
  analogWriteRange(PWM_RANGE_FULL);
  pinMode(WHITE_PIN, OUTPUT);


  //ledstrip.init(NUM_LEDS);
  setColour(0, 0, 0);

  Blynk.begin(auth, ssid, pass);
  Serial.println("Loaded following on startup");
  loadSettings();
  

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(ntpSyncInterval);
  digitalClockDisplay();
  Serial.println(currentSeconds());
  pixels.begin();
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
}
time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
  Blynk.run();


  // Run CRON algorithm, but only if we have sync'd to NTP

    if (!turnOffCompletely && shouldTurnOnToday() && currentSeconds() > settings.stopTimeInSecs) {
      if (animationState == ANIM_STATE_SUNRISE) {
        Serial.println("Start fadeout");
        animationState = ANIM_STATE_FADEOUT;
        animationStartTime = millis();
      }
    } else if (!turnOffCompletely && shouldTurnOnToday() && currentSeconds() > settings.startTimeInSecs) {
      if (animationState == ANIM_STATE_OFF) {
        Serial.println("Start sunrise");
        animationState = ANIM_STATE_SUNRISE;
        animationStartTime = millis();
      }
    }
  

   if (animationState == ANIM_STATE_SUNRISE && (millis() - lastAnimationUpdate > animationInterval)) {
    drawSunriseFrame();
    lastAnimationUpdate = millis();
  }
  if (animationState == ANIM_STATE_FADEOUT && (millis() - lastAnimationUpdate > animationInterval)) {
    if (drawFadeoutFrame()==0) {
      Serial.println("Finished fadeout");
      animationState = ANIM_STATE_OFF;
      animationStartTime = 0;
    }
    lastAnimationUpdate = millis();
  }

  if(DEBUG>0 && millis()%10000 == 0){
    Serial.println("DEBUG");
    printSettings();
     Serial.print("last ntp sync: ");
    Serial.println(lastNTPSync);
        Serial.print("Current time sec: ");
    Serial.println(currentSeconds());
    printSettings();
    Serial.print("Animation state: ");
    Serial.println(animationState);
    Serial.print("animationStartTime: ");
    Serial.println(animationStartTime);
        Serial.print("Last Animation update millis: ");
    Serial.println(lastAnimationUpdate);
    Serial.print("Current sunlight frame: ");
    Serial.println( (millis() - animationStartTime) * settings.fps / 1000);
    Serial.print("White offset: ");
    Serial.println( WHITE_OFFSET);
    Serial.print("White offset frame condition: ");
    Serial.print( SUNRISE_FRAMES);
    Serial.print( " - ");
    Serial.print( WHITE_OFFSET);
    Serial.print( " = ");
    Serial.print( SUNRISE_FRAMES - WHITE_OFFSET);
    Serial.print("PWM range: ");
    Serial.println( PWM_RANGE_FULL);
    Serial.print("R: ");
    Serial.print( pixels.getPixelColor(NUM_LEDS-1)& 0xFF000);
    Serial.print("G: ");
    Serial.print( pixels.getPixelColor(NUM_LEDS-1)& 0x00FF00);
    Serial.print("B: ");
    Serial.print( pixels.getPixelColor(NUM_LEDS-1)& 0x0000FF);
    Serial.print("Should turn on : ");
    Serial.print( shouldTurnOnToday());
    Serial.print("Should turn off : ");
    Serial.print( turnOffCompletely);
    Serial.println();
    }
  //   if (
  //    (millis() - lastNTPRequest > ntpSyncInterval) ||
  //    (((lastNTPSync == 0) || (millis() - lastNTPSync > ntpSyncInterval * 10)) && millis() - lastNTPRequest > 1000)
  //  ) {
  //    flash(2);
  //    lastNTPRequest = millis();
  //    sendNTPpacket();
  //    Serial.print("lastNTPRequest: ");
  //    Serial.println(lastNTPRequest);
  //  }
  //digitalClockDisplay();

}



