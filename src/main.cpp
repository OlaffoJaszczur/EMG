#include <Arduino.h>
#include <Servo.h>
#include <ArduinoFFT.h>

arduinoFFT fft;

Servo myservo;  // create servo object to control a servo

int emgPin = A0;  // analog input pin for the EMG sensor
int empRawPin = A1; // analog input pin for the EMG raw signal sensor
int emgValue = 0; // variable to store the value read from the sensor
int emgRawValue = 0; // variable to store the value read from the raw signal sensor
int threshold = 200; // threshold value to detect muscle activity
int fs = 9800; // sampling frequeancy
float pi = 3.14; // rounded PI

const int numReadings = 10; // number of readings to smooth

int readings[numReadings]; // the readings from the analog input
int readIndex = 0; // the index of the current reading
int total = 0; // the running total
int average = 0; // the average

const float alphaLowPass = 1/(1+(fs/(2*pi*400)));  // Low-pass filter constant, target 400hz
const float alphaHighPass = 1/(1+(fs/(2*pi*10))); // High-pass filter constant, target 10hz
float lowPassValue = 0;          // Variable to store low-pass filter output
float highPassValue = 0;         // Variable to store high-pass filter output
float previousHighPassValue = 0; // Variable to store previous high-pass filter output

const int rmsWindowSize = 5; // Size of the RMS window
float rmsValues[rmsWindowSize]; // Array to hold the last 'rmsWindowSize' rectified values
int rmsIndex = 0; // Index for the next RMS value
float sumOfSquares = 0; // Sum of the squares of the last 'rmsWindowSize' values

void setup() {
    myservo.attach(3); // attaches the servo on pin 9 to the servo object
    pinMode(emgPin, INPUT);
    pinMode(empRawPin, INPUT);
    Serial.begin(9600);

    // initialize all the readings to 0:
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = 0;
    }
}

void loop() {
    emgValue = analogRead(emgPin); // read the value from the sensor
    emgRawValue = analogRead(empRawPin); //read the value from the raw signal sensor
//   Serial.print(emgRawValue);
//    Serial.print(" ");

// Filtering part of project

    float rawValue = emgRawValue * 100;

    // Apply the low-pass filter
    lowPassValue = alphaLowPass * rawValue + (1 - alphaLowPass) * lowPassValue;

    // Apply the high-pass filter
    highPassValue = alphaHighPass * (previousHighPassValue + rawValue - lowPassValue);
    previousHighPassValue = highPassValue;

    // Send the filtered value to the serial monitor
    Serial.print(highPassValue);
    Serial.print(" ");

    // Full-Wave Rectification
    float rectifiedValue = abs(highPassValue);

    // Send the rectified value to the serial monitor
    Serial.print(rectifiedValue);
    Serial.print(" ");

    // Add the square of the rectified value to the sum
    sumOfSquares -= rmsValues[rmsIndex] * rmsValues[rmsIndex];
    rmsValues[rmsIndex] = rectifiedValue;
    sumOfSquares += rmsValues[rmsIndex] * rmsValues[rmsIndex];

    // Increment the RMS index and wrap if necessary
    rmsIndex = (rmsIndex + 1) % rmsWindowSize;

    // Calculate the RMS value
    float rmsValue = sqrt(sumOfSquares / rmsWindowSize);
    // Send the RMS value to the serial monitor
    Serial.print(rmsValue);
    Serial.print(" ");

    // Amplify the RMS for better visibility
    float rmsAmp = (rmsValue) * 50;
    Serial.print(rmsAmp);
    Serial.print(" ");


    Serial.print(emgValue);
    Serial.print(" ");

// Servo actuation part of project

    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = rmsValue;
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= numReadings) {
        // ...wrap around to the beginning:
        readIndex = 0;
    }

    // calculate the average:
    average = total / numReadings;
    // send it to the servo
    myservo.write(map(average, 0, 600, 0, 180));

    // print the average to the Serial Monitor:
    //Serial.print("Average analog reading: ");
    Serial.println(average);


    delay(1); // delay in between reads for stability
}