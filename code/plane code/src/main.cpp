// created by brodum in may 2026
// this is the code for the plane 
#include <Arduino.h>
#include <ESP32Servo.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Servo flapServoRight;
Servo flapServoLeft;
Servo backServoRight;
Servo backServoLeft;
Servo backServoUp;
//////////////////
Adafruit_BMP085 bmp;
//////////////////
Adafruit_MPU6050 mpu;
//////////////////
static const int RXPin = 1, TXPin = 2;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
//////////////////


#define HC12 Serial2
static const uint32_t GPSBaud = 4800;

void recivingData();
void sendingData();
void controlMovement(float speed, int angleRoll, int anglePitch, int angleYaw);

String dataRecived[10];
// button1 button2 button3 joystick1x joystick1y joystick2x joystick2y pushsw leftsw rightsw
#define but1 dataRecived[0]
#define but2 dataRecived[1]
#define but3 dataRecived[2]
#define joystick1x dataRecived[3]
#define joystick1y dataRecived[4]
#define joystick2x dataRecived[5]
#define joystick2y dataRecived[6]
#define planeSpeed dataRecived[7]
void setup() {

 Serial.begin(115200);
 HC12.begin(9600, SERIAL_8N1, 17, 18);
 ss.begin(GPSBaud);
 mpu.begin();
 bmp.begin();

 flapServoLeft.attach(7);
 flapServoRight.attach(5);
 backServoLeft.attach(10);
 backServoRight.attach(11);
 backServoUp.attach(12);
// make gyro start values constant so that they alaways face north(ill do it when i make the project)


 ledcAttach(4, 5000, 16);



}

void loop() {
  sendingData();
  recivingData();
  controlMovement(planeSpeed.toFloat(), joystick1x.toInt(), joystick2y.toInt(), joystick2x.toInt());
}


void sendingData()
{
  String data = "%";
  data += String(gps.location.lat()) + "#";  // 
  data += String(gps.location.lng()) + "#"; //
  data += String(gps.speed.kmph()) + "#"; //
  data += String(bmp.readAltitude()) + "#";//
  data += String(mpu.gyro.x()) + "#";//
  data += String(analogRead(13)) + "#";//
  data += String(bmp.readTemperature()) + "&";//
  // I could've also added gyroy and gyroz for calculating the speed on each vector but now I want to finnish this faster
  HC12.println(data);
}

void controlMovement(float speed, int angleRoll, int anglePitch, int angleYaw)
{
  //control for flaps
  switch(but2.toInt()){//flaps switch manual/automatic
    case 0:
    flapServoLeft.write(angleRoll);
    flapServoRight.write(180-angleRoll);
    break;
    case 1:
    flapServoLeft.write(90+mpu.gyro.x());
    flapServoRight.write(90-mpu.gyro.x());
    break;   
  }
  //control for backwings
    backServoLeft.write(anglePitch);
    backServoRight.write(anglePitch);
  //controls fo back up wing
    backServoUp.write(angleYaw);
  //motorpower
  switch(but1.toInt()){//motor power/ motor speed switch
    case 0:
      ledcWrite(4, map(speed, 0, 100, 0, 65535));
      break;
    case 1:
       if(speed < gps.speed.kmph()) ledcWrite(4, map(speed, 0, 100, 0, 65535)+map(gps.speed.kmph()-speed, 0, 100, 0, 65535));
       else if(speed > gps.speed.kmph()) ledcWrite(4, map(speed, 0, 100, 0, 65535)-map(speed-gps.speed.kmph(), 0, 100, 0, 65535));
       else if(speed == gps.speed.kmph()) ledcWrite(4, map(speed, 0, 100, 0, 65535));
       else ledcWrite(4, 0);
      break;
  }
}
void recivingData()
{
  String data;
  if (HC12.available())
  {
    data = HC12.readStringUntil('\n');
    int index = 0;
    while (data[index] != '%')
    {
      index++;
    }
    index++; // Move past the '%'
    for (int i = 0; i < 8; i++)
    {
      dataRecived[i] = data.substring(index, data.indexOf('#', index));
      // Serial.println(dataRecived[i]);
      if (data.indexOf('#', index) > data.indexOf('&', index))
      {
        dataRecived[i] = dataRecived[i].substring(0, dataRecived[i].indexOf('&'));
        // Serial.println(dataRecived[i]);
        break;
      }
      index = data.indexOf('#', index) + 1;
      // data = data.substring(data.indexOf('&')+1);
    }
  }
}