/******************************************************************
 * A LCD displayer for Pi_Track using an Arduino mini pro
 * Need an Adafruit Graphic LCD 84x48 - Nokia 5110 library
 * Yamin Xu 2015/8/1    
 * xuyamin3d@gmail.com 
 ******************************************************************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);

#include <Wire.h>
#define SLAVE_ADDRESS 0x04
int number = 0;
int state = -1;
int tx = 0;
int ty = 0;
int object = 0;

void setup() {

  //Serial.begin(9600);
  //Serial.println("i2c init...");
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  //Serial.println("i2c Ready!");

  //Serial.println("lcd init...");
  pinMode(2, OUTPUT);

  digitalWrite(2, HIGH);

  display.begin();
  display.setContrast(60);

  display.display(); 

  display.clearDisplay();   // clears the screen and buffer
  //Serial.println("lcd Ready!");
  // init done
  
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(BLACK);

  display.setCursor(0, 1);
  display.print(" Pi Tracker");
  display.setCursor(0, 16);
  display.print("Designed by");
  display.setCursor(0, 28);
  display.print("  YaminXu");
  display.setCursor(0, 40);
  display.print("July 28,2015");

  display.display();
  delay(6000);
  display.clearDisplay();
}

void loop() {
  if (state != -1)
  {
    //display.clearDisplay();
    display.setCursor(0, 0);
    String stringShow = ("X:" + String(tx));
    display.print(stringShow);
    display.setCursor(0, 10);
    stringShow = ("Y:" + String(ty));
    display.print(stringShow);
    display.setCursor(0, 40);
    if (object == 0)
    {
      stringShow = ("NO TAR");
      display.drawRect(tx / 6 + 42, 5 + ty / 5, 8, 8, 1);
    }
    if (object == 1)
    {
      stringShow = ("TRACK..");
      display.fillRect(tx / 6 + 42, 5 + ty / 5, 8, 8, 1);
    }
    if (object == 2)
    {
      stringShow = ("LOST");
      display.drawRect(tx / 6 + 42, 5 + ty / 5, 8, 8, 1);
    }

    display.drawLine(tx / 6 + 46, 0, tx / 6 + 46, 48, BLACK);
    display.drawLine(42, 9 + ty / 5, 83, 9 + ty / 5, BLACK);

    display.print(stringShow);
    display.display();
    delay(25);
    display.clearDisplay();
  }
  else
  {
    display.setCursor(0, 10);
    display.print("System is starting...");
    delay(25);
    display.display();
  }
}

void receiveData(int byteCount)
{
  while (Wire.available()) {
    number = Wire.read();
  }
  if (number == 255)
  {
    state = 0;
  }
  else
  {
    if (state == 1)
    {
      tx = number;
      //Serial.print("data received X : ");
      //Serial.println(tx);
    }
    if (state == 2)
    {
      ty = number;
      //Serial.print("data received Y : ");
      //Serial.println(ty);
    }
    if (state == 3)
    {
      object = number;
      //Serial.print("data received Obj : ");
      //Serial.println(object);
    }
  }
  state = state + 1;
}

void sendData() {
  Wire.write(number);
}

