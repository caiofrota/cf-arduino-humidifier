/**
 * CF Wi-Fi Humidifier.
 * 
 * IoT - Configurable humidifier using ESP8266 (ESP01 or NodeMCU 1.0 ESP-12E), DHT (11 or 22),
 * ultrasonic atomizer and ThingsBoard as server.
 * 
 * @author  Caio Frota <caiofrota@gmail.com>
 * @version 1.0.0
 * @since   Mar, 2021
 */

#define USE_DISPLAY                                                                                 // Comment this line if you won't use display.
#define USE_ROTARY                                                                                  // Comment this line if you won't use rotary encoder.

// Libraries.
#include <ArduinoJson.h>                                                                            // Arduino JSON.
#include <DHT.h>                                                                                    // DHT.
#include <Logger.h>                                                                                 // Logger.
#include <SPI.h>                                                                                    // SPI.
#include <ThingsBoard.h>                                                                            // Things Board.
#include <WiFiManager.h>                                                                            // Wi-Fi Manager.
#include <Wire.h>                                                                                   // Wire.

#include <CFRotaryEncoder.h>                                                                        // CF Rotary Encoder.
#include <CFIconSet.h>                                                                              // CF Icon Set for display.

// Libraries Display.
#ifdef USE_DISPLAY
    #include <Adafruit_GFX.h>                                                                       // Adafruit GFX.
    #include <Adafruit_SSD1306.h>                                                                   // Adafruit display.
#endif

// Application attributes.
const char* appSketch = "cf-arduino-humidifier";                                                    // Application sketch.
const char* appVersion = "1.0.0";                                                                   // Application version.

void setup() {
    // Start Serial Monitor.
    Serial.begin(115200);
    //Logger::setLogLevel(Logger::VERBOSE);
    
    // Start display, if declared.
    #ifdef USE_DISPLAY
        displayBegin();
    #endif
    
    dhtBegin();
    wifiManagerBegin();
    humidifierBegin();

    #ifdef USE_ROTARY
        rotaryBegin();
    #endif
}

void loop() {
    dhtLoop();                                                                                      // DHT Loop.
    wifiManagerLoop();                                                                              // Wi-Fi Manager loop.
    tbLoop();                                                                                       // Things Board loop.
    humidifierLoop();                                                                               // Humidifier.

    #ifdef USE_ROTARY
        rotaryLoop();                                                                               // Rotory encoder.
    #endif
    
    #ifdef USE_DISPLAY
        displayLoop();                                                                              // Display loop.
    #endif
}
