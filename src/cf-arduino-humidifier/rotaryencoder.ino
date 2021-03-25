#ifdef USE_ROTARY

// Rotary Pinout.
const int ROT_PIN_OUTPUT_A = D6;
const int ROT_PIN_OUTPUT_B = D0;
const int ROT_PIN_PUSH_BUT = D5;

// Custom rotary constants.
const int ROT_MAX_ATTRIBUTES   = 3;
const int ROT_ATTR_HUMID_VALUE = 0;
const int ROT_ATTR_WORK_TIME   = 1;
const int ROT_ATTR_DELAY_TIME  = 2;

// CF Rotary.
CFRotaryEncoder rotaryEncoder(ROT_PIN_OUTPUT_A, ROT_PIN_OUTPUT_B, ROT_PIN_PUSH_BUT);                // Rotary Encoder With Push Button.

// Custom rotary attributes.
int rotaryAttribute = 0;                                                                            // Attribute that defines what should be changed.
bool rotaryStartAP = false;                                                                         // Flag that indicates the device should turn AP mode on.
bool rotaryReset = false;                                                                           // Flag that indicates the device should be reseted.

/**
 * Interrupt method for avoiding change signal lost.
 */
void ICACHE_RAM_ATTR rotaryInterrupt() {
    rotaryEncoder.interrupt();
}

/**
 * Rotary setup.
 */
void rotaryBegin() {
    // Config Callbacks.
    rotaryEncoder.setBeforeRotaryChangeValueCallback(rotaryBeforeChangeValueCallback);
    rotaryEncoder.setAfterRotaryChangeValueCallback(rotaryAfterChangeValueCallback);
    rotaryEncoder.setPushButtonOnShortPressReachedCallback(rotaryOnShortPressReachedCallback);
    rotaryEncoder.setPushButtonOnLongPressReachedCallback(rotaryOnLongPressReachedCallback);
    rotaryEncoder.setPushButtonOnPressCallback(rotaryOnPressCallback);
    rotaryEncoder.setPushButtonOnShortPressCallback(rotaryOnShortPressCallback);
    rotaryEncoder.setPushButtonOnLongPressCallback(rotaryOnLongPressCallback);
    
    // Setup interrupt.
    attachInterrupt(ROT_PIN_OUTPUT_A, rotaryInterrupt, CHANGE);

    // Start rotary.
    rotaryEncoder.begin();
}

/**
 * Rotary loop.
 */
void rotaryLoop() {
    rotaryEncoder.loop();
}

/**
 * Go to next attribute.
 */
void rotaryNextAttribute() {
    // Next.
    rotaryAttribute++;
    if (rotaryAttribute >= ROT_MAX_ATTRIBUTES) {
        // If max is reached, return to 0.
        rotaryAttribute = 0;
    }
}

/**
 * Method called before rotary value change.
 * Load initial rotary value.
 */
void rotaryBeforeChangeValueCallback() {
    if (rotaryAttribute == ROT_ATTR_HUMID_VALUE) {
        rotaryEncoder.setValue(getWifiParamHumidTrigger().toInt());
    } else if (rotaryAttribute == ROT_ATTR_WORK_TIME) {
        rotaryEncoder.setValue(getWifiParamWorkingTime().toInt());
    } else if (rotaryAttribute == ROT_ATTR_DELAY_TIME) {
        rotaryEncoder.setValue(getWifiParamDelayTime().toInt());
    }
}

/**
 * Method called after rotary value change.
 * Persist new rotary value.
 */
void rotaryAfterChangeValueCallback() {
    if (rotaryAttribute == ROT_ATTR_HUMID_VALUE) {
        // Controlling values that are not between 0 and 100.
        rotaryEncoder.setValue(
                rotaryEncoder.getValue() > 100
                    ? 100
                    : (
                        rotaryEncoder.getValue() < 0
                            ? 0
                            : rotaryEncoder.getValue()
                     )
        );
        setWifiParamHumidTrigger(String(rotaryEncoder.getValue()));
    } else if (rotaryAttribute == ROT_ATTR_WORK_TIME) {
        setWifiParamWorkingTime(String(rotaryEncoder.getValue()));
    } else if (rotaryAttribute == ROT_ATTR_DELAY_TIME) {
        setWifiParamDelayTime(String(rotaryEncoder.getValue()));
    }
}

/**
 * Method called when push button's short time press time is recahed.
 */
void rotaryOnShortPressReachedCallback() {
    rotaryStartAP = true;
}

/**
 * Method called when push button's long time press time is recahed.
 */
void rotaryOnLongPressReachedCallback() {
    rotaryReset = true;
}

/**
 * Method called when push button is pressed.
 * Change the attribute.
 * 
 */
void rotaryOnPressCallback() {
    rotaryNextAttribute();
    Logger::error("Click ");
    Logger::error(getWifiParamHumidTrigger());
}

/**
 * Method called when push button is short time pressed.
 * Start AP config portal.
 */
void rotaryOnShortPressCallback() {
    rotaryStartAP = false;
    wifiStartConfigPortal();
}

/**
 * Method called when push button is long time pressed.
 * Reset device.
 */
void rotaryOnLongPressCallback() {
    rotaryStartAP = false;
    rotaryReset = false;
    resetConfig();
}

/**
 * Get rotary attribute.
 * 
 * @return Rotary attribute.
 */
int getRotaryAttribute() {
    return rotaryAttribute;
}

/**
 * Get if AP mode should be started.
 * 
 * @returns True if AP mode should be started.
 */
bool isRotaryStartAP() {
    return rotaryStartAP;
}

/**
 * Get if device should be reseted.
 * 
 * @returns True if device should be reseted.
 */
bool isRotaryReset() {
    return rotaryReset;
}

#endif
