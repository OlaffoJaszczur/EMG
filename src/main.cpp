#include <Arduino.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int emgPin = A0;  // analog input pin for the EMG sensor
int emgValue = 0; // variable to store the value read from the sensor
int threshold = 600; // threshold value to detect muscle activity
bool muscleConstricted = false; // flag to track muscle activity

unsigned long lastConstrictionTime = 0; // time when the last muscle constriction was detected
const unsigned long latency = 100; // latency in milliseconds (2000ms = 2 seconds)

int currentPosition = 0; // current position of the servo
int targetPosition = 0; // target position of the servo
const int maxPosition = 300; // maximum position of the servo
const int stepSize = 1; // step size for smooth movement

void setup() {
    myservo.attach(3); // attaches the servo on pin 9 to the servo object
    pinMode(emgPin, INPUT);
    Serial.begin(9600);
}

void loop() {
    emgValue = analogRead(emgPin); // read the value from the sensor
    Serial.println(emgValue);

    if (emgValue > threshold && !muscleConstricted) {
        lastConstrictionTime = millis(); // store the time of muscle constriction
        muscleConstricted = true;
        targetPosition = maxPosition; // set target position
    }

    if (millis() - lastConstrictionTime > latency && muscleConstricted) {
        // Gradually move to the target position
        if (currentPosition < targetPosition) {
            currentPosition += stepSize;
            myservo.write(currentPosition);
            delay(5); // delay for smoother transition
        } else {
            muscleConstricted = false; // reset the muscle activity flag
            targetPosition = 0; // reset target position
        }
    }

    // Gradually move back to the initial position
    if (!muscleConstricted && currentPosition > 0) {
        currentPosition -= stepSize;
        myservo.write(currentPosition);
        delay(5); // delay for smoother transition
    }
}

