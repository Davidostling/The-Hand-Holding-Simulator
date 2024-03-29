#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
const int buzzpin = 9;
const int buzzpin2 = 3;
const int buttonPin = 6;
float previousReading_fov = 0;  // Previous reading from the presence sensor (AK9753)
float previousReading = 180;
float lowpassReading_fov;  // Lowpass filtered readings from the presence sensor (AK9753)
float lowpassDistance;
int counter = 0;
//const int sound = A4;
int Array[2];

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
Servo myServo;
boolean buttonState = 0;
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  //pinMode(sound, INPUT);
  myServo.attach(5);
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  radio.setPALevel(RF24_PA_MAX);
  Serial.begin(9600);

  Serial.println("fruit VL53L0X test");
  if (!lox.begin(0x30)) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
}
void loop() {
  delay(5);
  
  radio.startListening();
  if ( radio.available()) {
    while (radio.available()) {
      int angleV[3];
      radio.read(&angleV, sizeof(angleV));

      
      int angle = (angleV[2]);

      lowpassReading_fov = 0.85 * previousReading_fov + 0.15* angle;
      previousReading_fov = lowpassReading_fov;

      int vinkel = previousReading_fov / 30;
      
      Serial.println(vinkel);

      
      myServo.write(vinkel);

      int ton = angleV[0];

      if (ton < 450 || ton >550){
        tone(buzzpin, ton);
      }
      else{
        noTone(buzzpin);
      }
      

    }
    delay(5);
    radio.stopListening();
    
    VL53L0X_RangingMeasurementData_t measure;
      
    //Serial.print("Reading a measurement... ");
    lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  
    if (measure.RangeStatus != 4) {  // phase failures have incorrect data
      Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
    } else {
      Serial.println(" out of range ");
    }
    int distance = measure.RangeMilliMeter;

    
    if (distance > 180){
      distance = 180;
    }
    lowpassDistance = 0.85 * previousReading + 0.15* distance;
    previousReading = lowpassDistance;
    
    buttonState = digitalRead(buttonPin);
    
    Array[0] = lowpassDistance;
    Array[1] = buttonState;
    radio.write(&Array, sizeof(Array));
    
  }

}