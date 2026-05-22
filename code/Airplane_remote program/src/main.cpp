/*Note from the guy that made this code:
This code is for controlling an airplane remotly using the HC12 Module and the ili9341 320x240 display

P.S this code works and you can control the plane with it but the data i send to the plane needs to be calibrated. So, this code is not finished because of these inconveniences. But i really want to do this RC plane so i am gonna update my github afer i get some first hand expirience with the plane. 
                                                                pushsw
                                                              ____|____
                                                   leftsw    --|     | -- rightsw
 /----------------|--------------------------------------------|-----|----------\
|                                                    -----   -----   -----      |      
|                         /---------------------\   | (_) | | (_) | | (_) |     |                               
|                         |                     |    -----   -----   -----      |     
|                         |                     |     but1    but2   but3       |         
|                         |                     |                               | 
|      __________         |                     |        __________             |           
|      |   __   |         |                     |        |   __   |             |             
|      |  (  )  |         |         TFT         |        |  (  )  |             |                    
|      |  (__)  |         |                     |        |  (__)  |             |          
|      |________|         |                     |        |________|             |           
|       joystick1         |                     |         joystick2             |
|                         |                     |                               |
|                         |                     |                               |
|                         \---------------------/                               |                     
|                                                                               |
 \------------------------------------------------------------------------------/
joystick1 = controls the plane's roll 
joystick2 = controls the plane's pitch and yaw
buttons = extra controls for the plane 
leftsw and rightsw = controls the plane's speed
pushsw = extra control for the plane




*/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SoftwareSerial.h>

#define DWIDTH 240
#define DHEIGHT 320
#define HC12 Serial2

// pin definitions 
#define RXHC12 17
#define TXHC12 18
#define but1 1
#define but2 2
#define but3 8
#define joystick1x 4
#define joystick1y 5
#define joystick2x 6
#define joystick2y 7
#define pushsw 12
#define rightsw 13
#define leftsw 14

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};
uint16_t *sprPtr[2];
String dataRecived[10];

#define planelat dataRecived[0]
#define planelong dataRecived[1]
#define planespeed dataRecived[2]
#define planealt dataRecived[3]
#define planeroll dataRecived[4]
#define planebattery dataRecived[5]
#define planetemp dataRecived[6]

int planeSpeed = 0;
void recivingData();
void sendingData();
void setingup();
void setup()
{
  
  HC12.begin(9600, SERIAL_8N1, 17, 18);
  Serial.begin(115200);
  bool x = false;
  tft.init();
  tft.initDMA();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  sprPtr[0] = (uint16_t *)spr[0].createSprite(DWIDTH, DHEIGHT / 2);
  sprPtr[1] = (uint16_t *)spr[1].createSprite(DWIDTH, DHEIGHT / 2);
  spr[1].setViewport(0, -DHEIGHT / 2, DWIDTH, DHEIGHT);
  // spr[0].setTextDatum(MC_DATUM);
  // spr[1].setTextDatum(MC_DATUM);
  uint16_t calData[5] = {568, 2923, 441, 3263, 0};
  tft.setTouch(calData);
  tft.startWrite();
  /////////////////////////////////////////////////////////////////////////
  setingup();
}
void UI(float bang = 0, int alt = 0, String longg = "0.000", String lat = "0.000", int temp = 0, int battery = 0)
{
  // spr[0].fillSprite(TFT_WHITE);
  float speed = 23.5;
  float distance = 0;
  int batteryLevel = 0;
  float ang = bang * (3.14 / 180);
  float sinAng = sin(ang);
  int x = 70, y = 60, r = 50;
  int f = int(float(r) * sinAng);
  int x1 = int(float(r) - float(r) * cos(ang));
  /////////////////////////////////////////////////////////////////////
  spr[0].fillSprite(TFT_BLACK);
  spr[0].drawCircle(x, y, r, TFT_WHITE);
  spr[0].fillCircle(x, y, 4, TFT_GREEN);
  spr[0].drawLine(x + 4, y - 2, x + r - 7, y - 2, TFT_WHITE);
  spr[0].drawLine(x + 4, y + 2, x + r - 7, y + 2, TFT_WHITE);
  spr[0].drawLine(x - 4, y - 2, x - r + 7, y - 2, TFT_WHITE);
  spr[0].drawLine(x - 4, y + 2, x - r + 7, y + 2, TFT_WHITE);
  spr[0].drawPixel(x + 3, y + 3, TFT_WHITE);
  spr[0].drawPixel(x - 3, y - 3, TFT_WHITE);
  spr[0].drawPixel(x - 3, y + 3, TFT_WHITE);
  spr[0].drawPixel(x + 3, y - 3, TFT_WHITE);
  spr[0].drawFastHLine(x - 2, y - 4, 5, TFT_WHITE);
  spr[0].drawFastHLine(x - 2, y + 4, 5, TFT_WHITE);
  /////////////////////////////////////////////////////////////////////
  spr[0].drawLine((x - r) + x1, y - f, (x + r) - x1, y + f, TFT_GREEN);
  spr[0].drawLine((x - r) + x1, y + 1 - f, (x + r) - x1, y + 1 + f, TFT_GREEN);
  spr[0].drawLine((x - r) + x1, y - 1 - f, (x + r) - x1, y - 1 + f, TFT_GREEN);
  /////////////////////////////////////////////////////////////////////
  spr[0].fillCircle(x, y, 2, TFT_BLACK);
  spr[0].setTextColor(tft.color565(53, 181, 141));
  spr[0].drawCentreString(String(int(bang)) + " deg", x, y - 30, 1);
  // spr[0].drawFastVLine(180, 20,100,TFT_WHITE);
  //////////////////////////////////////////////////////////////////////
  spr[0].drawFastVLine(x + r + 70, y - 50, 100, TFT_WHITE);
  spr[0].setTextColor(TFT_WHITE);
  for (int i = 0; i < 10; i++)
  {
    spr[0].drawFastHLine(x + r + 67, y + 50 - (i * 11), 7, TFT_WHITE);
    spr[0].drawNumber(i * 15, x + r + 74, y + 50 - (i * 11) - 3, 1);
  }
  spr[0].drawCentreString("m", 220, 0, 2);
  spr[0].drawCentreString(String(alt), x + r + 50, y + 50 - (alt * 11 / 15) - 3, 1);
  spr[0].drawFastHLine(x + r + 57, y + 50 - (alt * 11 / 15), 8, TFT_GREEN);
  /////////////////////////////////////////////////////////////////////
  spr[1].fillSprite(TFT_BLACK);
  int x2 = 10, y2 = 120, r2 = 57;
  spr[1].drawCentreString("Plane up for:", 120, y2 + r2 + 5, 2);
  spr[1].drawCentreString("20:33", 120, y2 + r2 + 25, 7);
  spr[1].setTextColor(TFT_GREEN);
  spr[1].drawCentreString("Connected", 120, y2 + r2 + 95, 4);
  spr[1].setTextColor(TFT_WHITE);
  for (int i = 0; i < 2; i++)
  {
    spr[i].drawRect(x2, y2, 130, r2, TFT_WHITE);
    spr[i].drawString("Long:" + longg + " deg", x2 + 5, y2 + 5, 1);
    spr[i].drawString("Lat:" + lat + " deg", x2 + 5, y2 + 15, 1);
    spr[i].drawString("Temperature:" + String(temp) + " C", x2 + 5, y2 + 25, 1);
    spr[i].drawString("Distance(FSP): " + String(distance), x2 + 5, y2 + 35, 1);
    spr[i].drawString("Battery: " + String(batteryLevel) + "%", x2 + 5, y2 + 45, 1);
    spr[i].drawString("Speed km/h:", x2 + 135, y2, 2);
    spr[i].drawCentreString(String(speed), x2 + 175, y2 + 25, 4);
    tft.pushImageDMA(0, i * DHEIGHT / 2, DWIDTH, DHEIGHT / 2, sprPtr[i]);
  }
}
long long unsigned int t1 = millis();
int fps = 0;
void loop()
{
  UI(planeroll.toFloat(), planealt.toInt(), planelong, planelat, planetemp.toInt(), planebattery.toInt());
  uint16_t x = 0, y = 0;
  bool pressed = tft.getTouch(&x, &y);
  if (pressed)
  {
    tft.fillCircle(x, y, 10, TFT_RED);
    Serial.printf("Touch at %d,%d\n", x, y);
  }
  recivingData();
  if(digitalRead(leftsw) == 0 && millis()-t1 > 50){
    planeSpeed--;
    t1 = millis();
  }
  else if (digitalRead(rightsw) == 0 && millis()-t1 > 50){
    planeSpeed++;
    t1 = millis();
  }

  // fps++;
  // if(millis() - t1 > 1000){
  //   Serial.println("FPS: " + String(fps));
  //   fps = 0;
  //   t1 = millis();
  // }
}
void setingup()
{

  for (int i = 0; i < 2; i++)
  {
    spr[i].fillSprite(TFT_BLACK);
    spr[i].setTextColor(TFT_RED);
    spr[i].drawCentreString("Not Connected", 120, 120, 4);
    spr[i].drawCentreString("Waiting for Plane...", 120, 150, 2);
    tft.pushImageDMA(0, i * DHEIGHT / 2, DWIDTH, DHEIGHT / 2, sprPtr[i]);
  }
  while (true)
  {
    if (Serial.available())
    {
      break;
    }
  }
  for (int i = 0; i < 2; i++)
  {
    spr[i].fillSprite(TFT_BLACK);
    spr[i].setTextColor(TFT_YELLOW);
    spr[i].drawCentreString("Setting starting coordinates...", 120, 120, 2);
    spr[i].drawCentreString("Long: ", 120, 150, 2);
    spr[i].drawCentreString("Lat: ", 120, 170, 2);
    tft.pushImageDMA(0, i * DHEIGHT / 2, DWIDTH, DHEIGHT / 2, sprPtr[i]);
    spr[i].setTextColor(TFT_WHITE);
  }
  delay(4000);
  while (true)
  {
    if (Serial.available())
    {
      break;
    }
  }
}
void sendingData()
{
  String data = "%";
  data += String(digitalRead(but1)) + "#";  // button1
  data += String(digitalRead(but2)) + "#";  // button2
  data += String(digitalRead(but3)) + "#";  // button3
  data += String(map(digitalRead(joystick1x),0,4095,0,180)) + "#";  // joystick1
  data += String(map(digitalRead(joystick1y),0,4095,0,180)) + "#";  // joystick1
  data += String(map(digitalRead(joystick2x),0,4095,0,180)) + "#";  // joystick2
  data += String(map(digitalRead(joystick2y),0,4095,0,180)) + "#";  // joystick2
  data += String(planeSpeed) + "&";  // speed
  HC12.println(data);
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
    for (int i = 0; i < 7; i++)
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