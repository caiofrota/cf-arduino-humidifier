#ifdef USE_DISPLAY

#define CF_DISPLAY_WIDTH                128                                                         // Display width.
#define CF_DISPLAY_HEIGHT               64                                                          // Display height.
#define CF_DISPLAY_ADDRESS              0x3C                                                        // Display I2C address.

Adafruit_SSD1306 display(CF_DISPLAY_WIDTH, CF_DISPLAY_HEIGHT, &Wire, -1);                           // Display.

void displayRenderAPPage() {
    
    display.clearDisplay();

    // Draw bitmaps.
    display.drawBitmap(0, 0, CFIconSet::NETWORK_HIGH_BARS_8X8, 8, 7, 1);                            // Network.
    display.drawBitmap(96, 0, CFIconSet::PHONE_8X8, 8, 7, 1);                                       // Things Board.

    // Draw lines.
    display.setCursor(0, 0);                                                                        // Line 1 Size 1
    display.print("  AP STARTED      OFF");

    display.setCursor(0, 24);                                                                       // Line 3 Size 1
    display.print(" SSID: " + getWiFiDefaultSSID());

    display.setCursor(0, 32);                                                                       // Line 4 Size 1
    display.print(" PASS: " + getWiFiDefaultPass());
    
    display.setCursor(0, 40);                                                                       // Line 5 Size 1
    display.print(" IP: " + getWiFiAPIP());
    
    display.display();
}

void displayRenderMainPage() {
    display.clearDisplay();
    display.cp437(true);

    if (!isRotaryReset()) {                                                                         // Check if the device sohuld be reseted and do not display
        // Draw bitmaps.                                                                            // anything if True.
        display.drawBitmap(0, 0, CFIconSet::NETWORK_HIGH_BARS_8X8, 8, 7, 1);                        // Network.
        display.drawBitmap(96, 0, CFIconSet::PHONE_8X8, 8, 7, 1);                                   // Things Board.
        display.drawBitmap(8, 24, CFIconSet::THERMOMETER_8X8, 8, 7, 1);                             // Temperature.
        display.drawBitmap(8, 40, CFIconSet::WATERDROP_8X8, 8, 7, 1);                               // Humidity.
        display.drawBitmap(64, 40, CFIconSet::SHOWERS_8X8, 8, 7, 1);                                // Trigger.
        
        // Draw lines.
        display.setCursor(0, 0);                                                                    // Line 1 Size 1.
        if (!isWiFiConnected()) {
            display.print("  OFFLINE         OFF");
        } else {
            String displaySSID = getWiFiSSID().substring(0, 13);
            for (int i = displaySSID.length(); i < 13; i++) {
                displaySSID += " ";
            }
            display.print("  " + displaySSID + "   ");
            if (isTBConnected()) {
                display.print("ON");
            } else {
                display.print("OFF");
            }
            
            display.setCursor(0, 8);                                                                // Line 2 Size 1.
            display.print("IP: " + getWiFiIP());
        }
        
        String tempC = String(getDHTTemperatureC());
        tempC = tempC.substring(0, tempC.lastIndexOf(".") + 2);
        for (int i = tempC.length(); i < 5; i++) {
            tempC += " ";
        }
        display.setCursor(0, 24);                                                                   // Line 4 Size 1.
        display.print("   ");
        display.print(tempC);
        display.write(248);
        display.print("C   ");
        
        String tempF = String(getDHTTemperatureF());
        tempF = tempF.substring(0, tempF.lastIndexOf(".") + 2);
        for (int i = tempF.length(); i < 5; i++) {
            tempF += " ";
        }
        display.print(tempF);
        display.write(248);
        display.print("F");
        
        String humidity = String(getDHTHumidity());
        humidity = humidity.substring(0, humidity.lastIndexOf(".") + 2);
        for (int i = humidity.length(); i < 5; i++) {
            humidity += " ";
        }
        display.setCursor(0, 40);                                                                   // Line 5 Size 1.
        display.print("   ");
        display.print(humidity);
        display.print("%    ");
        
        int rotAttr = getRotaryAttribute();
        if (rotAttr == 0) {
            String trigger = String(getWifiParamHumidTrigger());
            for (int i = trigger.length(); i < 3; i++) {
                trigger = " " + trigger;
            }
            display.print(trigger);
            display.print("  %");
        } else if (rotAttr == 1) {
            String workingTime = String(getWifiParamWorkingTime());
            for (int i = workingTime.length(); i < 3; i++) {
                workingTime = " " + workingTime;
            }
            display.print(workingTime);
            display.print("  WT");
        } else if (rotAttr == 2) {
            String delayTime = String(getWifiParamDelayTime());
            for (int i = delayTime.length(); i < 3; i++) {
                delayTime = " " + delayTime;
            }
            display.print(delayTime);
            display.print("  DT");
        }
    
        if (isRotaryStartAP()) {                                                                    // If the button is pressed for a short time. AP mode.
            display.setCursor(0, 56);
            display.print("                   AP");
        }
    }

    
    display.display();
}

/**
 * Start display.
 */
void displayBegin() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally.
    if(!display.begin(SSD1306_SWITCHCAPVCC, CF_DISPLAY_ADDRESS)) {
        Logger::error("SSD1306 allocation failed");
        for(;;);                                                                                    // Don't proceed, loop forever.
    }
    
    //display.display();                                                                              // Adafruit Logo.
    //delay(2000);                                                                                    // Pause for 2 seconds.

    // Display logo.
    display.clearDisplay();
    display.drawBitmap(0, 0, CFIconSet::CFLOGO_128X64, 128, 64, 1);
    display.display();
    delay(3000);

    // Clear display.
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
}

/**
 * Dispaly loop.
 */
void displayLoop() {
    displayRenderMainPage();
}

#endif
