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

const int numReadings = 10; // number of readings to smooth

int readings[numReadings]; // the readings from the analog input
int readIndex = 0; // the index of the current reading
int total = 0; // the running total
int average = 0; // the average

const uint16_t samples = 256; // Number of samples for FFT. Must be a power of 2
const double samplingFrequency = 1000; // Sampling frequency in Hz

double vReal[samples];
double vImag[samples];

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
    Serial.print(emgRawValue);
    Serial.print(" ");
//    Serial.println(emgValue);


//    myservo.write(map(analogRead(emgPin),0,1023,0,180));
//    delay(100);

    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = analogRead(emgValue);
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
//    Serial.print("Average analog reading: ");
//    Serial.println(average);

    for(int i = 0; i < samples; i++) {
        vReal[i] = analogRead(empRawPin); // Read the sensor
        vImag[i] = 0; // Set imaginary part to 0
        delayMicroseconds(1000000 / 9600);
    }

    // Apply FFT
    fft.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft.Compute(vReal, vImag, samples, FFT_FORWARD);
    fft.ComplexToMagnitude(vReal, vImag, samples);

    Serial.println(vReal);

    delay(1); // delay in between reads for stability
}
