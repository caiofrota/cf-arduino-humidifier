/**
 * DHT Sensor - Digital Humidity and Temperature Sensor.
 * 
 * Libraries:
 *      - DHT Sensor Library Version 1.4.1 (https://github.com/adafruit/DHT-sensor-library/releases/tag/1.4.1).
 * 
 * Components:
 *      DHT11 or DHT22 module (Not tested with other DHT type).
 * 
 * DHT Acknowledged BUG: https://github.com/adafruit/DHT-sensor-library/issues/94
 * 
 *      It was noticed that DHT stop working after some uptime, after a long research about that
 *      was figured that it's a acknowleged BUG with no solution so far. So i've implemented
 *      a workaround until this problem is definitelly solved.
 *      
 * Workaround:
 *      A pin is being used for physically restarting DHT when it's getting NaN.
 */

// DHT Pins.
const int DHT_PIN_DATA    = 2;            // (GPIO2 / D4 - NodeMCU)                                 // DHT Pin Data.
const int DHT_PIN_RESTART = 0;            // (GPIO0 / D3 - NodeMCU)                                 // DHT Pin RESTART (Workaround for DHT reading failure).

// DHT Constants.
const int DHT_TIME_TO_READ  = 2000;       // Default 2 seconds.                                     // Delay time between readings.

// DHT Object.
//DHT dht(DHT_PIN_DATA, DHT11);                                                                       // DHT11.
DHT dht(DHT_PIN_DATA, DHT22);                                                                       // DHT22.

// DHT Control.
bool dhtFirstLoop = true;                                                                           // Flag that indicates if it is the first loop.

// DHT attributes.
float dhtTemperatureC = 0;                                                                          // Temperature in Celsius.
float dhtTemperatureF = 0;                                                                          // Temperature in Fahrenheit.
float dhtHumidity     = 0;                                                                          // Humidity.
float dhtHeatIndexC   = 0;                                                                          // Heat index in Celsius.
float dhtHeatIndexF   = 0;                                                                          // Heat index in Fahrenheit.
long  dhtLastRead     = 0;                                                                          // Store last time data was read.
bool  dhtRead         = false;                                                                      // Flag for read failure checking.

/**
 * Collect DHT data.
 */
void dhtReadData() {
    if (dhtLastRead == 0 || millis() - dhtLastRead > DHT_TIME_TO_READ) {
        dhtLastRead = millis();
        
        // Read DHT
        dhtTemperatureC = dht.readTemperature();
        dhtTemperatureF = dht.readTemperature(true);
        dhtHumidity = dht.readHumidity();
    
        // Check if it was read.
        if (isnan(dhtTemperatureC) || isnan(dhtTemperatureF) || isnan(dhtHumidity)) {
            Logger::warning("Failed to read from DHT sensor!");
            dhtTemperatureC = 0.0;
            dhtTemperatureF = 0.0;
            dhtHumidity = 0.0;
            dhtHeatIndexC = 0.0;
            dhtHeatIndexF = 0.0;
            dhtRead = false;
            digitalWrite(DHT_PIN_RESTART, !digitalRead(DHT_PIN_RESTART));                           // DHT Workaround for fail reading failure.
                                                                                                        // Physically power recycle.
            return;
        }
        
        dhtTemperatureC = roundf(dhtTemperatureC * 10) / 10;
        dhtTemperatureF = roundf(dhtTemperatureF * 10) / 10;
        dhtHumidity = roundf(dhtHumidity * 10) / 10;
        dhtHeatIndexC = roundf(dht.computeHeatIndex(dhtTemperatureC, dhtHumidity, false) * 10) / 10;
        dhtHeatIndexF = roundf(dht.computeHeatIndex(dhtTemperatureF, dhtHumidity) * 10) / 10;
        dhtRead = true;
    }
}

/**
 * Start DHT.
 */
void dhtBegin() {
    pinMode(DHT_PIN_RESTART, OUTPUT);                                                               // DHT Workaround for fail reading failure.
    dht.begin();
}

/**
 * DHT Loop.
 */
void dhtLoop() {
    dhtReadData();
}

/**
 * Get DHT Temperature in C.
 */
float getDHTTemperatureC() {
    return dhtTemperatureC;
}

/**
 * Get DHT Temperature in F.
 */
float getDHTTemperatureF() {
    return dhtTemperatureF;
}

/**
 * Get DHT Heat Index in C.
 */
float getDHTHeatIndexC() {
    return dhtHeatIndexC;
}

/**
 * Get DHT Heat Index in F.
 */
float getDHTHeatIndexF() {
    return dhtHeatIndexF;
}

/**
 * Get DHT Humidity.
 */
float getDHTHumidity() {
    return dhtHumidity;
}

/**
 * Get DHT Read.
 * 
 * @returns True if dht was read successfully.
 */
bool isDHTRead() {
    return dhtRead;
}
