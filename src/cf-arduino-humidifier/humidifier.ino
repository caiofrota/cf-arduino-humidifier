const int HMD_PIN_FOGGER = D7;
const int HMD_PIN_FAN    = D8;

bool humidifierIsWorking = false;                                                                   // Flag that indicates if the humidifier is on.
bool humidifierFanIsWorking = false;                                                                // Flag that indicates if the humidifier is on.
long humidifierLastWork = 0;                                                                        // Last start millis.
long humidifierFanLastWork = 0;                                                                     // Last fan start time.
int humidifierTrigger = 0;                                                                          // Trigger value.
long humidifierWorkTime = 0;                                                                        // Active time.
long humidifierDelayTime = 0;                                                                       // Time to wait for next reading.

void humidifierBegin() {
    pinMode(HMD_PIN_FAN, OUTPUT);
    pinMode(HMD_PIN_FOGGER, OUTPUT);
    digitalWrite(HMD_PIN_FOGGER, LOW);
    digitalWrite(HMD_PIN_FAN, LOW);
}

void humidifierLoop() {
    humidifierTrigger = getWifiParamHumidTrigger().toInt();
    humidifierWorkTime = getWifiParamWorkingTime().toInt() * 1000; // Read stored time in seconds.
    humidifierDelayTime = getWifiParamDelayTime().toInt() * 1000; // Read stored time in seconds.
    
    if (getWifiParamHumidTrigger().toInt() != humidifierTrigger) {
        setWifiParamHumidTrigger(String(humidifierTrigger));
    }
    if (humidifierFanIsWorking) {
        if (millis() - humidifierFanLastWork > humidifierWorkTime + (humidifierWorkTime > 60000 ? 30000 : 5000)) {
                                                                                                    // Fan should work couple seconds more than the fogger.
            digitalWrite(HMD_PIN_FAN, LOW);
            humidifierFanIsWorking = false;
        }
    }
    if (humidifierIsWorking) {
        if (millis() - humidifierLastWork > humidifierWorkTime) {
            digitalWrite(HMD_PIN_FOGGER, LOW);
            humidifierIsWorking = false;
        }
    } else {
        if (humidifierLastWork == 0 || millis() - humidifierLastWork > humidifierDelayTime) {
            if (getDHTHumidity() <= humidifierTrigger) {
                digitalWrite(HMD_PIN_FAN, HIGH);
                digitalWrite(HMD_PIN_FOGGER, HIGH);
                humidifierLastWork = millis();
                humidifierFanLastWork = millis();
                humidifierIsWorking = true;
                humidifierFanIsWorking = true;
            }
        }
    }
}

bool isFoggerOn() {
    return humidifierIsWorking;
}

int setHumidifierTrigger(int trigger) {
    return humidifierTrigger = trigger;
}
