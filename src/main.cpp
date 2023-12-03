#include <Arduino.h>
#include <Servo.h>

template <int order> // order is 2 or...
class BandPass
{
private:
    float a[order+1];
    float b[order+1];
    float omega0;
    float Q;
    float domega;
    float dt;
    bool adapt;
    float tn1 = 0;
    float x[order+1]; // Raw values
    float y[order+1]; // Filtered values

public:
    BandPass(float f0, float fw, float fs, bool adaptive){
        // f0: central frequency (Hz)
        // fw: bandpass width (Hz)
        // fs: sample frequency (Hz)
        // adaptive: boolean flag, if set to 1, the code will automatically set
        // the sample frequency based on the time history.
        omega0 = 6.28318530718*f0;
        domega = 6.28318530718*fw;
        Q = omega0/domega;
        dt = 1.0/fs;
        adapt = adaptive;
        tn1 = -dt;
        for(int k = 0; k < order+1; k++){
            x[k] = 0;
            y[k] = 0;
            a[k] = 0;
            b[k] = 0;
        }
        setCoef();
    }

    void setCoef(){
        if(adapt){
            float t = micros()/1.0e6;
            dt = t - tn1;
            tn1 = t;
        }
        float alpha = omega0*dt;
        if(order==2){
            float D = pow(alpha,2) + 2*alpha/Q + 4;
            b[0] = 2*alpha/(Q*D);
            b[1] = 0;
            b[2] = -b[0];
            a[0] = 0;
            a[1] = -(2*pow(alpha,2) - 8)/D;
            a[2] = -(pow(alpha,2) - 2*alpha/Q + 4)/D;
        }
        else if(order==4){

        }
    }
    float filt(float xn){
        // Provide me with the current raw value: x
        // I will give you the current filtered value: y
        if(adapt){
            setCoef(); // Update coefficients if necessary
        }
        y[0] = 0;
        x[0] = xn;
        // Compute the filtered values
        for(int k = 0; k < order+1; k++){
            y[0] += a[k]*y[k] + b[k]*x[k];
        }

        // Save the historical values
        for(int k = order; k > 0; k--){
            y[k] = y[k-1];
            x[k] = x[k-1];
        }

        // Return the filtered value
        return y[0];
    }
};

Servo myservo;  // create servo object to control a servo

int emgPin = A0;  // analog input pin for the EMG sensor
int empRawPin = A1; // analog input pin for the EMG raw signal sensor
int emgValue = 0; // variable to store the value read from the sensor
int emgRawValue = 0; // variable to store the value read from the raw signal sensor
int threshold = 200; // threshold value to detect muscle activity
int fs = 9800; // sampling frequeancy Hz
float pi = 3.14; // rounded PI

const int numReadings = 10; // number of readings to smooth

int readings[numReadings]; // the readings from the analog input
int readIndex = 0; // the index of the current reading
int total = 0; // the running total
int average = 0; // the average

const float LowPassTarget = 500; //Target 400hz
const float HighPassTarget = 5; //Target 10hz
const float CentralFrequency = sqrt(LowPassTarget*HighPassTarget);
const float Bandwidth = LowPassTarget-HighPassTarget;
// Filter instance
BandPass<2> lp(CentralFrequency,Bandwidth, fs,true);

const int rmsWindowSize = 7; // Size of the RMS window
float rmsValues[rmsWindowSize]; // Array to hold the last 'rmsWindowSize' rectified values
int rmsIndex = 0; // Index for the next RMS value
float sumOfSquares = 0; // Sum of the squares of the last 'rmsWindowSize' values

float emgRmsWindow[rmsWindowSize]; // Array to store the recent samples
int emgRmsIndex = 0; // Index for the next sample in the RMS window
float emgSumOfSquares = 0; // Sum of the squares of the values in the window

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
    Serial.println("emgRawValue,rectifiedValue,rmsValue,emgValue, emgRmsValue");
    Serial.print(emgRawValue);
    Serial.print(" ");

// Filtering part of project

    float rawValue = emgRawValue*100;

    // Apply the band-pass filter
    float bandPassValue = lp.filt(rawValue);  // Variable to store band-pass filter output

    // Send the filtered value to the serial monitor
    Serial.print(bandPassValue);
    Serial.print(" ");

    // Full-Wave Rectification
    float rectifiedValue = abs(bandPassValue);

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

    // Update the RMS window and sum of squares
    emgSumOfSquares -= emgRmsWindow[emgRmsIndex] * emgRmsWindow[emgRmsIndex]; // Remove the oldest squared value
    emgRmsWindow[emgRmsIndex] = emgValue; // Insert the new value
    emgSumOfSquares += emgRmsWindow[emgRmsIndex] * emgRmsWindow[emgRmsIndex]; // Add the new squared value

    // Increment the RMS index and wrap if necessary
    emgRmsIndex = (emgRmsIndex + 1) % rmsWindowSize;

    // Calculate the RMS value
    float emgRmsValue = sqrt(emgSumOfSquares / rmsWindowSize);

    // Use rmsValue for further processing or output
    Serial.print(emgValue);
    Serial.print(" ");
    Serial.println(emgRmsValue);

// Servo actuation part of project

    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = emgRmsValue;
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
//    Serial.println(average);


    delay(1); // delay in between reads for stability
}