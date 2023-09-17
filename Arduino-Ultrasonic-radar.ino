#include <SD.h>
#include <Wire.h>
#include <Servo.h>
#include "RTClib.h"

#define trigPin 8
#define echoPin 9

RTC_PCF8523 rtc;

uint32_t syncTime = 0;

File logfile;
long duration;
int distance;
Servo myservo;

int calculateDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    return distance;
}

void error(char const *str)
{
    Serial.print("error: ");
    Serial.println(str);

    while (1)
        ;
}
void createLogFile()
{
    //Create a new file
    char filename[] = "MLOG00.CSV";
    for (uint8_t i = 0; i < 100; i++)
    {
        filename[4] = i / 10 + '0';
        filename[5] = i % 10 + '0';
        if (!SD.exists(filename))
        {
            logfile = SD.open(filename, FILE_WRITE);
            break;
        }
    }

    if (!logfile)
    {
        error("Failed to create file");
    }

    // Serial.print("Logging to: ");
    // Serial.println(filename);
}
void setup()
{
    Serial.begin(9600);

    //Initialize SD card
    //Serial.println("Initializing SD card...");
    pinMode(10, OUTPUT);
    if (!SD.begin(10))
    {
        Serial.println("SD card initialization failed.");
        return;
    }

    // Serial.println("card initialized.");

    createLogFile();

    // Initialize RTC module
    Wire.begin();
    if (!rtc.begin())
    {
        logfile.println("RTC failed");
        Serial.println("Couldn't find RTC");
        // while (1);
    }

    //Initialize the ultrasonic sensor
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    myservo.attach(7);
   

    // Output title row
    logfile.println("millis,stamp,datetime,degree,distance");
    // Serial.println("millis,stamp,datetime,degree,distance");
}

int startDegree = 0;
int endDegree = 180;

void loop()
{

    int i;

    int u = 1; // Control the step of degree change
    // /Increasing from 0 degrees to 180 degrees, and then decreasing.This cycle is repeated.
    for (i = startDegree; i <= endDegree + 1; i = i + u)
    {
        if (i >= endDegree + 1)
        {
            i = i - 2;
            u = -u;
        }
        if (i <= startDegree - 1)
        {
            i += 2;
            u = -u;
        }
        myservo.write(i);
        delay(15);

        calculateDistance();
        Serial.print(i);
        Serial.print(",");
        Serial.print(distance);
        Serial.print(".");

        if ((999 - millis() % 1000) >= 960)
        // Record once every 1000 milliseconds (1 second)
        // This is an infinite loop. I try to omit all the steps of function calling,
        // which can minimize the error and make the interval between recording data as close as possible to 1000 milliseconds,
        // but the error is inevitable.
        {
            DateTime now = rtc.now();
            logfile.print(millis());
            logfile.print(",");
            logfile.print(now.unixtime());
            logfile.print(",");
            logfile.print(now.year(), DEC);
            logfile.print("/");
            logfile.print(now.month(), DEC);
            logfile.print("/");
            logfile.print(now.day(), DEC);
            logfile.print(" ");
            logfile.print(now.hour(), DEC);
            logfile.print(":");
            logfile.print(now.minute(), DEC);
            logfile.print(":");
            logfile.print(now.second(), DEC);
            logfile.print(",");
            logfile.print(i);
            logfile.print(",");
            logfile.println(distance);
            // Serial.println(millis());
            logfile.flush();
        }
    }
}
