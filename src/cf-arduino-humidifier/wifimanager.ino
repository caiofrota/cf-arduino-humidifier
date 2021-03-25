#define WIFI_CUSTOM_PARAM_SIZE          6                                                           // Custom parameters array size.

// Custom Parameters.
#define WIFI_P_DEVICE_NAME              0                                                           // Custom parameter index.
#define WIFI_P_SERVER_URL               1                                                           // Custom parameter index.
#define WIFI_P_TOKEN                    2                                                           // Custom parameter index.
#define WIFI_P_HUMID_TRGR               3                                                           // Custom parameter index.
#define WIFI_P_WORKING_TIME             4                                                           // Custom parameter index.
#define WIFI_P_DELAY_TIME               5                                                           // Custom parameter index.
WiFiManagerParameter wifiManagerCustomParameters[] = {                                              // Custom paramaters array.
    { "p_device_name", "Device Name", "", 50 },
    { "p_server_url", "Server URL", "", 50 },
    { "p_server_token", "Token", "", 50 },
    { "p_humid_trgr", "Trigger", "35", 50 },                                                        // Default: 35% of humidity.
    { "p_humid_work", "Working Time", "600", 50 },                                                  // Default: 10 minutes.
    { "p_humid_delay", "Delay Time", "1800", 50 }                                                   // Default: 20 minutes.
};

WiFiManager wifiManager;                                                                            // Wi-Fi Manager.
WiFiServer wifiServer(80);                                                                          // Wi-Fi.

const String WIFI_DEFAULT_SSID = wifiManager.getDefaultAPName();                                    // Default SSID.
const String WIFI_DEFAULT_PASSWORD = "12345678";                                                    // Default Wi-Fi password.
bool wifiConnected = false;                                                                         // True if Wi-Fi is connected.
String wifiSSID = wifiManager.getDefaultAPName();                                                   // Wi-Fi SSID.
String wifiAPIP = "";                                                                               // Wi-Fi AP IP.
String wifiIP = "";                                                                                 // Wi-Fi IP address.

/**
 * Start Wi-Fi Manager.
 */
void wifiManagerBegin() {
    //SPIFFS.format();                                                              // Format SPIFFS. Uncomment this line to format SPIFFS at boot.
    //wifiManager.resetSettings();                                                  // Reset config. Uncomment this line to reset settings at boot.
    
    // Load Wi-Fi custom parameters.
    wifiLoadCustomParameters();
    
    // Add params.
    for (int i = 0; i < WIFI_CUSTOM_PARAM_SIZE; i++) {
        wifiManager.addParameter(&wifiManagerCustomParameters[i]);
    }

    // Config Wi-Fi Manager.
    wifiManager.setAPCallback(wifiConfigModeCallback);
    wifiManager.setSaveParamsCallback(wifiSaveCustomParameters);
    std::vector<const char *> menu = { "wifi"                                                       // Config Wi-Fi.
                                     , "param"                                                      // Config Params.
                                     , "info"                                                       // Wi-Fi info.
                                     , "update"                                                     // Firmware update.
                                     , "sep"                                                        // Separator.
                                     , "restart"                                                    // Restart device.
                                     , "exit"                                                       // Exit portal.
                                     , "sep"                                                        // Separator.
                                     , "erase"};                                                    // Erase device data.
    wifiManager.setMenu(menu);
    wifiManager.setConfigPortalTimeout(30);                                                         // Auto close config portal timeout.
    wifiManager.setClass("invert");                                                                 // Dark theme.
    
    // Start Wi-Fi Manager.
    WiFi.mode(WIFI_STA);
    bool res = wifiManager.autoConnect(WIFI_DEFAULT_SSID.c_str(), WIFI_DEFAULT_PASSWORD.c_str());
    
    if (res) {
        // Connected.
        wifiSSID = WiFi.SSID();
        wifiIP = WiFi.localIP().toString();
        wifiManager.startWebPortal();
        wifiServer.begin();
        wifiConnected = true;
    } else {
        // Not connected.
    }
}

/**
 * Wi-Fi Manager loop.
 */
void wifiManagerLoop() {
    wifiManager.process();
}

/**
 * Config mode callback.
 * 
 * @param in  WiFiManager *myWiFiManager: Wi-Fi Manager.
 */
void wifiConfigModeCallback(WiFiManager *myWiFiManager) {
    wifiAPIP = WiFi.softAPIP().toString();
    #ifdef USE_DISPLAY
        displayRenderAPPage();
    #endif
}

/**
 * Start config portal.
 */
void wifiStartConfigPortal() {
    #ifdef USE_DISPLAY
        displayRenderAPPage();
    #endif
    bool res = wifiManager.startConfigPortal(WIFI_DEFAULT_SSID.c_str(), WIFI_DEFAULT_PASSWORD.c_str());
    
    if (res) {
        // Connected.
        wifiSSID = WiFi.SSID();
        wifiIP = WiFi.localIP().toString();
        wifiManager.startWebPortal();
        wifiServer.begin();
        wifiConnected = true;
    } else {
        // Not connected.
    }
}

/**
 * Reset config.
 */
void resetConfig() { 
    //SPIFFS.format();                                                              // Format SPIFFS.
    wifiManager.erase        ();                                                    // Erase Wi-Fi credentials.
    wifiManager.resetSettings();                                                    // Reset config.
    ESP.restart();
}

/**
 * Load Wi-Fi Custom Parameters.
 */
void wifiLoadCustomParameters() {
    Logger::notice("Mount FS.");
    if (SPIFFS.begin()) {
        Logger::notice("Loading Wi-Fi custom parameters.");
        
        // Open file for reading.
        String filePath = "/config.json";
        if (SPIFFS.exists(filePath)) {
            Logger::notice("Open file for reading.");
            File file = SPIFFS.open(filePath, "r");
            if (file) {
                // Create JSON objects.
                DynamicJsonDocument doc(1024);
                
                // Deserialize file into JSON object.
                DeserializationError error = deserializeJson(doc, file);
                if (error) {
                    Logger::error(String("Fail deserializing file. Code: ") + error.c_str());
                }
                
                // Set custom parameters.
                for (int i = 0; i < WIFI_CUSTOM_PARAM_SIZE; i++) {
                    String key = wifiManagerCustomParameters[i].getID();
                    if (doc.containsKey(key)) {
                        String value = doc[key];
                        wifiManagerCustomParameters[i].setValue(value.c_str(), wifiManagerCustomParameters[i].getValueLength());
                    }
                }
                Logger::notice("Custom parameters loaded.");
                
                // Close file.
                file.close();
            } else {
                Logger::error("Fail opening file.");
            }
        }
    } else {
        Logger::error("Fail mounting FS.");
    }
}

/**
 * Save Wi-Fi Custom Parameters.
 */
void wifiSaveCustomParameters() {
    // Load values from Wi-Fi Manager form.
    Logger::notice("Saving Wi-Fi custom parameters.");
    
    // Open file for writing.
    String filePath = "/config.json";
    Logger::notice("Open file for writing.");
    File file = SPIFFS.open(filePath, "w");
    if (file) {
        // Create JSON objects.
        DynamicJsonDocument doc(1024);
        
        // Set custom pameters.
        for (int i = 0; i < WIFI_CUSTOM_PARAM_SIZE; i++) {
            doc[wifiManagerCustomParameters[i].getID()] = wifiManagerCustomParameters[i].getValue();
        }
        
        if (serializeJson(doc, file) == 0) {
            Logger::error("Fail serializing file.");
        } else {
            Logger::notice("Custom parameters saved.");
        }
        
        // Close file.
        file.close();
    } else {
        Logger::notice("Fail opening file.");
    }
}

/**
 * Get if wifi is connected.
 * 
 * @return bool: True if Wi-Fi is connected.
 */
bool isWiFiConnected() {
    return wifiConnected;
}

/**
 * Get Wi-Fi default SSID.
 * 
 * @return String: Wi-Fi SSID.
 */
String getWiFiDefaultSSID() {
    return WIFI_DEFAULT_SSID;
}

/**
 * Get Wi-Fi SSID.
 * 
 * @return String: Wi-Fi SSID.
 */
String getWiFiSSID() {
    return wifiSSID;
}

/**
 * Get Wi-Fi default password.
 * 
 * @return String: Wi-Fi default password.
 */
String getWiFiDefaultPass() {
    return WIFI_DEFAULT_PASSWORD;
}

/**
 * Get IP.
 * 
 * @return String: Wi-Fi IP.
 */
String getWiFiAPIP() {
    return wifiAPIP;
}

/**
 * Get IP.
 * 
 * @return String: Wi-Fi IP.
 */
String getWiFiIP() {
    return wifiIP;
}

/**
 * Get server URL.
 * 
 * @return String: Server URL.
 */
String getWifiParamServerURL() {
    return (String) wifiManagerCustomParameters[WIFI_P_SERVER_URL].getValue();
}

/**
 * Get token.
 * 
 * @return String: Token.
 */
String getWifiParamToken() {
    return (String) wifiManagerCustomParameters[WIFI_P_TOKEN].getValue();
}

/**
 * Get device name.
 * 
 * @return String: Device Name.
 */
String getWifiParamDeviceName() {
    return (String) wifiManagerCustomParameters[WIFI_P_DEVICE_NAME].getValue();
}

/**
 * Get Humidifier Trigger.
 * 
 * @return String: Humidifier Trigger.
 */
String getWifiParamHumidTrigger() {
    return (String) wifiManagerCustomParameters[WIFI_P_HUMID_TRGR].getValue();
}

/**
 * Get working time.
 * 
 * @return String: Working time.
 */
String getWifiParamWorkingTime() {
    return (String) wifiManagerCustomParameters[WIFI_P_WORKING_TIME].getValue();
}

/**
 * Get delay time.
 * 
 * @return String: Delay time.
 */
String getWifiParamDelayTime() {
    return (String) wifiManagerCustomParameters[WIFI_P_DELAY_TIME].getValue();
}

/**
 * Define device name.
 * 
 * @param in  String value: Device name.
 */
void setWifiParamDeviceName(String value) {
    wifiManagerCustomParameters[WIFI_P_DEVICE_NAME].setValue(value.c_str(), wifiManagerCustomParameters[WIFI_P_DEVICE_NAME].getValueLength());

    // Call save parameters.
    wifiSaveCustomParameters();
}

/**
 * Define humidifier trigger.
 * 
 * @param in  String value: Humidifier Trigger.
 */
void setWifiParamHumidTrigger(String value) {
    wifiManagerCustomParameters[WIFI_P_HUMID_TRGR].setValue(value.c_str(), wifiManagerCustomParameters[WIFI_P_HUMID_TRGR].getValueLength());
    setHumidifierTrigger(value.toInt());

    // Call save parameters.
    wifiSaveCustomParameters();
}

/**
 * Define working time.
 * 
 * @param in  String value: Working time.
 */
void setWifiParamWorkingTime(String value) {
    wifiManagerCustomParameters[WIFI_P_WORKING_TIME].setValue(value.c_str(), wifiManagerCustomParameters[WIFI_P_WORKING_TIME].getValueLength());

    // Call save parameters.
    wifiSaveCustomParameters();
}

/**
 * Define delay time.
 * 
 * @param in  String value: Delay time.
 */
void setWifiParamDelayTime(String value) {
    wifiManagerCustomParameters[WIFI_P_DELAY_TIME].setValue(value.c_str(), wifiManagerCustomParameters[WIFI_P_DELAY_TIME].getValueLength());

    // Call save parameters.
    wifiSaveCustomParameters();
}
