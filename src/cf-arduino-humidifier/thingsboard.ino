WiFiClient wifiClient;                                                                              // Wi-Fi.
ThingsBoard thingsBoard(wifiClient);                                                                // Things Board.

// Attributes.
bool tbConnected = false;                                                                           // True if Things Board is connected.
bool rpcSubscribed = false;                                                                         // True if application is subscribed with RPC.
bool attrSubscribed = false;                                                                        // True if application is subscribed with attributes.
long thingsBoardTimeToSend = 60000;    // 60 seconds.                                               // Things Board minimum frequency to send data.
long thingsBoardTimeToRetry = 10000;   // 10 seconds.                                               // Things Board minimum frequency to retry connecting.
long lastSentMillis = 0;                                                                            // Store last time data was sent to server.

/**
 * Send data to Things Board.
 */
void tbSendData() {
    // JSON Data.
    DynamicJsonDocument json(64);

    // Seding DHT reading status.
    json["dht_fogg"] = isFoggerOn();
    json["dht_read"] = isDHTRead();
    char isRead[64]; serializeJson(json, isRead);
    thingsBoard.sendTelemetryJson(isRead);

    // Sending DHT data.
    json.clear();
    json["dht_tempc"] = getDHTTemperatureC();
    json["dht_tempf"] = getDHTTemperatureF();
    json["dht_humid"] = getDHTHumidity();
    char dht[64]; serializeJson(json, dht);
    
    thingsBoard.sendTelemetryJson(dht);

    json.clear();
    json["dht_hidxc"] = getDHTHeatIndexC();
    json["dht_hidxf"] = getDHTHeatIndexF();
    char heat[64]; serializeJson(json, heat);
    
    thingsBoard.sendTelemetryJson(heat);
}

/**
 * Send attributes to Things Board.
 */
void tbSendAttrData() {
    thingsBoard.sendAttributeString("attr_device_name", getWifiParamDeviceName().c_str());
    thingsBoard.sendAttributeString("attr_device_humidtrgr", getWifiParamHumidTrigger().c_str());
    thingsBoard.sendAttributeString("attr_device_workingtime", getWifiParamWorkingTime().c_str());
    thingsBoard.sendAttributeString("attr_device_delaytime", getWifiParamDelayTime().c_str());
}

/** 
 * Things Board attribute change callback.
 */
void tbReceiveAttrCallback(const RPC_Data &data) {
    Logger::notice("Received the set delay ATTR method.");
    
    // Device Name.
    if (data["p_device_name"]) {
        setWifiParamDeviceName(data["p_device_name"]);
    }

    // Humidifier Trigger.
    if (data["attr_device_humidtrgr"]) {
        setWifiParamHumidTrigger(data["p_humid_trgr"]);
    }

    // Working time.
    if (data["attr_device_workingtime"]) {
        setWifiParamWorkingTime(data["p_humid_work"]);
    }

    // Delay time.
    if (data["attr_device_delaytime"]) {
        setWifiParamDelayTime(data["p_humid_delay"]);
    }
}

/** 
 * Things Board Default RPC method callback.
 */
void tbReceiveRPCDefaultCallback(const RPC_Data &data, RPC_Response &resp) {
    Logger::notice("Received the set delay RPC method.");
    
    // Process data.
    StaticJsonDocument<200> doc;
    deserializeJson(doc, data);
    
    int value = doc["value"];
    
    // Return value.
    JsonObject r  = resp.to<JsonObject>();
    r["value"] = value;
}

/** 
 * Things Board Default RPC method callback.
 */
void tbGetHumidityTriggerCallback(const RPC_Data &data, RPC_Response &resp) {
    Logger::notice("Received the set delay RPC method.");
    
    // Process data.
    StaticJsonDocument<200> doc;
    doc["value"] = getWifiParamHumidTrigger().toInt();
    
    // Return value.
    resp = doc["value"];
}

/** 
 * Things Board Default RPC method callback.
 */
void tbSetHumidityTriggerCallback(const RPC_Data &data, RPC_Response &resp) {
    Logger::notice("Received the set delay RPC method.");
    
    // Process data.
    int value = data;
    setWifiParamHumidTrigger(String(value));
    
    // Return value.
    resp = data;
}

// Things Board RPC callback method list.
int tbReceiveRPCCallbackListSize = 3;
RPC_Callback tbReceiveRPCCallbackList[] = {
    { "default",                    tbReceiveRPCDefaultCallback },
    { "getHumidityTriggerValue",    tbGetHumidityTriggerCallback },
    { "setHumidityTriggerValue",    tbSetHumidityTriggerCallback }
};

/**
 * Things Board loop.
 */
void tbLoop() {
    if (!thingsBoard.connected()) {
        if (lastSentMillis == 0 || (millis() - lastSentMillis) > thingsBoardTimeToRetry) {
            tbConnected = false;
            rpcSubscribed = false;
            attrSubscribed = false;
            char serverURL[50]; strcpy(serverURL, getWifiParamServerURL().c_str());
            char token[50]; strcpy(token, getWifiParamToken().c_str());
            char deviceName[50]; strcpy(deviceName, getWifiParamDeviceName().c_str());
            char humidTrigger[50]; strcpy(humidTrigger, getWifiParamHumidTrigger().c_str());
            char workingTime[50]; strcpy(workingTime, getWifiParamWorkingTime().c_str());
            char delayTime[50]; strcpy(delayTime, getWifiParamDelayTime().c_str());
            
            Logger::notice("Connecting to Things Board node.");
            if (thingsBoard.connect(serverURL, token)) {
                tbConnected = true;
                
                // Processing data.
                char espChipId[6];
                sprintf(espChipId, "%06X", ESP.getChipId());
                
                // Send application attributes.
                thingsBoard.sendAttributeString("app_sketch", appSketch);
                thingsBoard.sendAttributeString("app_version", appVersion);
                thingsBoard.sendAttributeString("device_chip_id", espChipId);
                thingsBoard.sendAttributeString("device_local_ip", WiFi.localIP().toString().c_str());
                thingsBoard.sendAttributeString("attr_device_name", deviceName);
                thingsBoard.sendAttributeString("attr_device_humidtrgr", humidTrigger);
                thingsBoard.sendAttributeString("attr_device_workingtime", humidTrigger);
                thingsBoard.sendAttributeString("attr_device_delaytime", delayTime);
            } else {
                Logger::warning("Fail connecting Things Board. Retrying in " + String(thingsBoardTimeToRetry / 1000) + " second(s).");
                lastSentMillis = millis();
                return;
            }
        } else {
            return;
        }
    }
    
    if (lastSentMillis == 0 || millis() - lastSentMillis > thingsBoardTimeToSend) {
        Logger::notice("Sending data to Things Board.");
        tbSendData();
        Logger::notice("Sending attribute data to Things Board.");
        tbSendAttrData();
        lastSentMillis = millis();
    }
    
    // RPC subscription.
    if (!rpcSubscribed) {
        Logger::notice("Subscribing for RPC.");
        if (!thingsBoard.RPC_Subscribe(tbReceiveRPCCallbackList, tbReceiveRPCCallbackListSize)) {
          Logger::error("Fail subscribing for RPC.");
          return;
        }
        rpcSubscribed = true;
    }
    
    // Attribute subscription.
    if (!attrSubscribed) {
        Logger::notice("Subscribing for attributes.");
        if (!thingsBoard.Attr_Subscribe(tbReceiveAttrCallback)) {
          Logger::error("Fail subscribing for attributes.");
          return;
        }
        attrSubscribed = true;
    }

    // Call Things Board loop.
    thingsBoard.loop();
}

/**
 * Check if Things Board is connected.
 * 
 * @return bool: True if Things Board is connected.
 */
bool isTBConnected() {
    return tbConnected;
}
