#include <Arduino.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int emgPin = A0;  // analog input pin for the EMG sensor
int emgValue = 0; // variable to store the value read from the sensor
int threshold = 500; // threshold value to detect muscle activity

void setup() {
    myservo.attach(9); // attaches the servo on pin 9 to the servo object
    pinMode(emgPin, INPUT);
    Serial.begin(9600);
}

void loop() {
    emgValue = analogRead(emgPin); // read the value from the sensor

    // Print the value to the Serial Monitor and Plotter
    Serial.println(emgValue);

    if (emgValue > threshold) {
        // if muscle activity detected
        myservo.write(30); // sets the servo position according to the scaled value
    } else {
        myservo.write(0); // reset servo position
    }

    delay(10); // wait for the servo to get there
}
