/* Paper display controller for ESP8266 */
/* Brian Whitman brian@variogr.am */

#include <epd.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <HttpClient.h>

/* Auth.h has the following things in it:
const char WiFiSSID[] = "your wifi SSID";
const char WiFiPSK[] = "your wifi password";
const char hostname[] = "hostname.of.your.webservice.com";
const char path[] = "hostname.of.your.webservice.com/any_path";
*/

#include "auth.h"

// How long to sleep in between displays, in seconds
int sleepTimeS = 60*60; // one hour
#define LED_PIN 5 // ESP8266 LED PIN 
#define WAKEUP_PIN 4 // ESP8266 pin that will wake up the EPD -- not to be confused with XPO/DTR to wake up the ESP
#define MAX_H_CHARS 55 // Max characters to show horizontally
char result[1000]; // Buffer to get web results in from

// Backup object to store in EEPROM in between updates 
// Since the MCU turns off, we have to store any state we need 
// in a struct and save that to EEPROM
// For this device, we actually don't need any state, but just
// to show you how to do it, I store a counter that increments
// every time the display is updated.
struct BackupObj {
  int counter;
};
BackupObj b;

// Update the EEPROM with the contents of the struct in b
void updateEEPROM() {
  int eeAddress = 0;
  EEPROM.begin(16);  //set up memory allocation
  EEPROM.put( eeAddress, b );
  EEPROM.commit();
  EEPROM.end();
}

// Fill the struct b with the contents of the EEPROM
void readEEPROM() {
  int eeAddress = 0; 
  EEPROM.begin(16);
  EEPROM.get( eeAddress, b );
  EEPROM.end();
}

// This is called every wake up
void setup() {
  b.counter = 0;
  // Turn the LED on during network communications if you want
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  connectWiFi();
  // Before turning on the EPD, get the message from the webservice
  getMessage();
  // Update the backup struct 
  readEEPROM();
  // Set up the EPD display
  epd_init(WAKEUP_PIN); 
  epd_wakeup();
  epd_set_memory(MEM_NAND);
}

// draw a message to the EPD 
void drawMessage(void) {
  int i = 0;
  int j = 0;
  int v = 10;
  char buf[200];
  int buf_idx = 0;
  
  digitalWrite(LED_PIN, HIGH);
  epd_wakeup();
  epd_set_color(BLACK, WHITE); 
  epd_clear();
  // You can load other fonts if you want using the SD card in back
  epd_set_en_font(ASCII64);
  // This is a poor person' word wrap
  for(i=0;i<strlen(result);i++) {
    if(result[i] == '\n' || result[i] == '\r' || result[i] == 0 || buf_idx == MAX_H_CHARS) { 
      for(j=buf_idx;j<MAX_H_CHARS;j++) buf[j] = 0;
      // For each line incremenent vertical position and draw the text at 10 px in
      epd_disp_string(buf, 10, v);
      v = v + 80;
      buf_idx = 0;
    } else {
      buf[buf_idx++] = result[i];
    }
  }
  // Clear out the buffer with 0s to not freak out the EPD
  for(j=buf_idx;j<MAX_H_CHARS;j++) buf[j] = 0;
  // Draw the lat line
  epd_disp_string(buf, 10, v);
  epd_udpate();
  // Sleep the EPD
  epd_enter_stopmode();
  b.counter = b.counter + 1;
  updateEEPROM();
  digitalWrite(LED_PIN, LOW);  
}

void loop() {
  // This is only called once per wake up period
  drawMessage();  
  // Sleep the ESP is ultra low power, will call setup() on boot again
  ESP.deepSleep(sleepTimeS * 1000000);
  // Code wil never get here...
  //
}



void connectWiFi() {
  byte ledStatus = LOW;
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    delay(100);
  }
}

// Connect to a text only webservice and store the result
void getMessage() {
  digitalWrite(LED_PIN, HIGH);

  WiFiClient client;
  HttpClient http(client);
  char url[255]; 
  http.get(hostname, path);
  http.skipResponseHeaders();
  int count = 0;
  while(http.available()) {
    char c = http.read();
    if(count < 1000) result[count++] = c;
  }
  // clear out the rest
  for(int i=count;i<1000;i++) result[i] = 0;
  digitalWrite(LED_PIN, LOW);
}
