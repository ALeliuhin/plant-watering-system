#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>  
#include <LiquidCrystal_I2C.h> 

// Initialization of LCD, GPS & Serial pins
LiquidCrystal_I2C LCD(0x27, 16, 2); // Initialize the LCD with I2C address 0x27, 16 columns and 2 rows
TinyGPSPlus GPS; // Create a TinyGPS++ object
SoftwareSerial mySerial(4, 3); // RX pin = 4, TX pin = 3

// Auxiliary variables for control
uint8_t relayPin = 8;
uint8_t motorSpeedPin = 6;
uint8_t directionPin1 = 12;
uint8_t directionPin2 = 13;

int gpsArray[3] = {0, 0, 0}; // {hours, minutes, seconds}
unsigned long previousMillis = 0; // Stores the last time millis() was called
unsigned long motorTimeMillis = 0;
bool gpsTimeInitialized = false; // Flag to track whether valid GPS time has been obtained
bool motorTimeFlag = true;

bool gpsFlagArray[2] = {0, 0};

void countTime(int array[3], unsigned long duration) {
    unsigned long currentMillis = millis(); // Get the current time
    previousMillis = millis();  // Reset previousMillis at the start

    while (millis() - currentMillis < duration && gpsTimeInitialized == true) {
        if (millis() - previousMillis >= 1000UL) { // Check if a second has passed
            previousMillis = millis();
        
            // Increment the seconds counter
            array[2] = (array[2] + 1) % 60;
        
            // If seconds roll over to 0, increment the minutes counter
            if (array[2] == 0) {
                array[1] = (array[1] + 1) % 60;
            
                // If minutes roll over to 0, increment the hours counter
                if (array[1] == 0) {
                    array[0] = (array[0] + 1) % 24; // Use 24-hour format
                }
            }

            // Update the LCD display with the updated time
            LCD.setCursor(6, 0);   // Set cursor to the 7th column on the first row
            LCD.print("Time");     // Display "Time" on the LCD

            LCD.setCursor(3, 1);   // Set cursor to the second row, 2nd column
            LCD.print(array[0]);   // Print hour value
            LCD.print(":");

            LCD.setCursor(7, 1);   // Set cursor to the second row, 6th column
            if (array[1] < 10) LCD.print("0"); // Add leading zero for minutes if needed
            LCD.print(array[1]);   // Print minute value
            LCD.print(":");

            LCD.setCursor(11, 1);   // Set cursor to the second row, 10th column
            if (array[2] < 10) LCD.print("0"); // Add leading zero for seconds if needed
            LCD.print(array[2]);
        }

        // Trigger the relay if time is 14:38:00 and if relay hasn't been triggered yet
        if (array[0] == 1 && array[1] == 33) {
            if(motorTimeFlag) motorTimeMillis = millis(); motorTimeFlag = false;
            if(millis() - motorTimeMillis < 30000UL){
                LCD.setCursor(0, 1);
                LCD.print("W");
                digitalWrite(relayPin, LOW);
            }
            else{
                LCD.clear();
                digitalWrite(relayPin, HIGH);
            }
        }
        else{
            motorTimeFlag = true;
        }
    }
}

void setup() {
    mySerial.begin(9600); // Start communication with the GPS module at 9600 baud rate

    LCD.init();           // Initialize the LCD
    LCD.backlight();       // Turn on the backlight

    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH);

    pinMode(motorSpeedPin, OUTPUT);
    analogWrite(motorSpeedPin, 255);
    pinMode(directionPin1, OUTPUT);
    digitalWrite(directionPin1, LOW);
    pinMode(directionPin2, OUTPUT);
    digitalWrite(directionPin2, HIGH);
}

void loop() {
    // Continuously read from the GPS module
    while (mySerial.available() > 0) {
        GPS.encode(mySerial.read()); // Encode the incoming GPS data

        // Check if time data is valid and set initial time
        if (GPS.time.isValid()) { 
            if(gpsArray[1] != GPS.time.minute())
                gpsArray[0] = (GPS.time.hour() + 3) % 24;  // Get hour with timezone offset (+3 hours)
                gpsArray[1] = GPS.time.minute();           // Get minute
                gpsArray[2] = GPS.time.second();           // Get second
                gpsTimeInitialized = true;
        }
    }

    // Only start counting time if GPS time has been initialized
    if (gpsTimeInitialized) {
        countTime(gpsArray, 60000); // Call countTime for 1 minute (60000 ms)
        gpsTimeInitialized = false;
    }
}



