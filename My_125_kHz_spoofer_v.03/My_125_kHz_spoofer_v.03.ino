#include <stdio.h>
#include <EEPROM.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Пин к которому подключена катушка - пин А5 (19 цифровой)
#define COIL 2



byte facility[2] = { 0x02, 0x0C };
byte cardID[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
int colsum[4] = { 0, 0, 0, 0}; // storage for the column checksums

// delay between symbols when we are transmitting
int bittime = 256;

byte RFIDdata[128];

int clock = 0; // storage for the current state of our clock signal.

byte datapointer = 0;
byte state;

//******************************************************
#define METAKOM_CYFRAL 0xFFFFFFFFFF
#define METAKOM_1 0x365A1140BE
#define CYFRAL_1 0x01FFFFFFFF
#define VIZIT_1 0x565A1140BE
#define VIZIT_2 0x365A398149
#define ELTIS 0x0
#define LIFT 0x0B57814601
#define TOILET 0x0400711335 //53

//uint64_t universalID[8] = {1099511627775, 233439314110, 8589934591, 370878267582, 233441952073, 0, 48712730113, 5675447};
uint64_t universalID[] =
{
  METAKOM_CYFRAL,
  METAKOM_1,
  CYFRAL_1,
  VIZIT_1,
  VIZIT_2,
  0,
  LIFT,
  TOILET
};
uint32_t nibleMask = 15;

#define BTN_UP   3
#define BTN_DWN  5
#define BTN_SEL  4

int8_t keyNumber = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DWN, INPUT);
  pinMode(BTN_SEL, INPUT);

  pinMode(COIL, OUTPUT);
  digitalWrite(COIL, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop(void)
{
  Serial.println(keyNumber);

  if (digitalRead(BTN_UP) == HIGH)
  {
    keyNumber++;
    if (keyNumber == 8) keyNumber = 0;
    while (digitalRead(BTN_UP) == HIGH);
    display.clearDisplay();
  }

  if (digitalRead(BTN_DWN) == HIGH)
  {
    keyNumber--;
    if (keyNumber == -1) keyNumber = 7;
    while (digitalRead(BTN_DWN) == HIGH);
    display.clearDisplay();
  }

  switch (keyNumber)
  {
    case 0:
      display.setCursor(0, 0);
      display.println(">1 MEATACOM_CYFRAL");
      display.println(" 2 MEATACOM_1");
      display.println(" 3 CYFRAL_1");
      display.println(" 4 VIZIT_1");
      display.display();
      break;
    case 1:
      display.setCursor(0, 0);
      display.println(" 1 MEATACOM");
      display.println(">2 MEATACOM_1");
      display.println(" 3 CYFRAL_1");
      display.println(" 4 VIZIT_1");
      display.display();
      break;
    case 2:
      display.setCursor(0, 0);
      display.println(" 1 MEATACOM");
      display.println(" 2 MEATACOM_1");
      display.println(">3 CYFRAL_1");
      display.println(" 4 VIZIT_1");
      display.display();
      break;
    case 3:
      display.setCursor(0, 0);
      display.println(" 1 MEATACOM");
      display.println(" 2 MEATACOM_1");
      display.println(" 3 CYFRAL_1");
      display.println(">4 VIZIT_1");
      display.display();
      break;
    case 4:
      display.setCursor(0, 0);
      display.println(">5 VIZIT_2");
      display.println(" 6 ELTIS");
      display.println(" 7 LIFT");
      display.println(" 8 TOILET");
      display.display();
      break;
    case 5:
      display.setCursor(0, 0);
      display.println(" 5 VIZIT_2");
      display.println(">6 ELTIS");
      display.println(" 7 LIFT");
      display.println(" 8 TOILET");
      display.display();
      break;
    case 6:
      display.setCursor(0, 0);
      display.println(" 5 VIZIT_2");
      display.println(" 6 ELTIS");
      display.println(">7 LIFT");
      display.println(" 8 TOILET");
      display.display();
      break;
    case 7:
      display.setCursor(0, 0);
      display.println(" 5 VIZIT_2");
      display.println(" 6 ELTIS");
      display.println(" 7 LIFT");
      display.println(">8 TOILET");
      display.display();
      break;
  }

  if (digitalRead(BTN_SEL) == HIGH)
  {
    display.clearDisplay();
    switch (keyNumber)
    {
      case 0:
        display.setCursor(0, 0);
        display.println("> MEATACOM_CYFRAL <");
        break;
      case 1:
        display.setCursor(0, 0);
        display.println("> MEATACOM_1 <");
        break;
      case 2:
        display.setCursor(0, 0);
        display.println("> CYFRAL_1 <");
        break;
      case 3:
        display.setCursor(0, 0);
        display.println("> VIZIT_1 <");
        break;
      case 4:
        display.setCursor(0, 0);
        display.println("> VIZIT_2 <");
        break;
      case 5:
        display.setCursor(0, 0);
        display.println("> ELTIS <");
        break;
      case 6:
        display.setCursor(0, 0);
        display.println("> LIFT <");
        break;
      case 7:
        display.setCursor(0, 0);
        display.println("> TOILET <");
        break;
    }
    display.println("Press reset");
    display.println("to continue...");
    display.display();

    facility[0] = universalID[keyNumber] >> 36 & nibleMask;
    facility[1] = universalID[keyNumber] >> 32 & nibleMask;

    for (uint8_t i = 0; i < 8; ++i ) //забиваем десятичный ID карты в массив
    {
      cardID[i] = (universalID[keyNumber] >> ((7 - i) * 4)) & nibleMask; //в нулевой элемент массива надо записывать старший разряд, поэтому 7-i
    }

    EmulateCard();  // start card emulation
  }

}
