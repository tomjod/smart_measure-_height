// RRE font example
// (C)2019 Pawel A. Hernik

#define LCD_RST 6
#define LCD_CS  7
#define LCD_BL  8

#include "HX1230_FB.h"
#include <SPI.h>
HX1230_FB lcd(LCD_RST, LCD_CS);

#include "RREFont.h"
#include "rre_logo.h"
#include "rre_chip.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

int sx=32, sy=32,sx0=0, sy0=0;
// special callback with scale&translation feature
void customRectScale(int x, int y, int w, int h, int c)
{
  int xx=(x-sx0)*sx>>5, yy=(y-sy0)*sy>>5, ww=((w+x-sx0)*sx>>5)-xx, hh=((h+y-sy0)*sy>>5)-yy; 
  xx+=sx0;
  yy+=sy0;
  return lcd.fillRect(xx, yy, ww, hh, c); 
}

void setup() 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  lcd.init();
  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
}

void bmp1()
{
  lcd.cls();
  font.setFont(&rre_logo); font.setSpacing(0);
  font.printStr(0,0," !");
  lcd.display();
  delay(100);
}

void bmp2()
{
  lcd.cls();
  font.setFont(&rre_chip); font.setSpacing(0); font.setSpacingY(0);
  font.printStr(0,0," !\"#$%\n&'()*+\n,-./01");
  font.printStr(16,16*3,"234567");
  lcd.display();
  delay(100);
}

void loop()
{
  font.setFillRectFun(customRect);
  lcd.cls();
  font.setFont(&rre_logo); font.setSpacing(0);
  font.printStr(0,0," !");
  lcd.display();
  delay(2000);

  lcd.cls();
  font.setFont(&rre_logo); font.setSpacing(1);
  font.printStr(0,0," !");
  lcd.display();
  delay(3000);

  lcd.cls();
  font.setFont(&rre_chip); font.setSpacing(0); font.setSpacingY(0);
  font.printStr(0,0," !\"#$%\n&'()*+\n,-./01");
  font.printStr(16,16*3,"234567");
  lcd.display();
  delay(2000);

  lcd.cls();
  font.setFont(&rre_chip); font.setSpacing(1); font.setSpacingY(1);
  font.printStr(0,0," !\"#$%\n&'()*+\n,-./01");
  font.printStr(16,17*3,"234567");
  lcd.display();
  delay(3000);

  int sc;
  font.setFillRectFun(customRectScale);
  sx0=SCR_WD/2; sy0=SCR_HT/2; // center
  for(sc=32;sc>0;sc--)  { sx=sy=sc; bmp2(); }
  for(sc=0;sc<=32;sc++) { sx=sy=sc; bmp1(); }
  delay(500);

  sx0=SCR_WD/2; sy0=0; // top
  for(sc=32;sc>0;sc--)  { sy=sc; sx=32; bmp1(); }
  for(sc=0;sc<=32;sc++) { sy=sc; sx=32; bmp2(); }
  delay(500);

  sx0=0; sy0=SCR_HT/2; // left
  for(sc=32;sc>0;sc--)  { sx=sc; sy=32; bmp2(); }
  for(sc=0;sc<=32;sc++) { sx=sc; sy=32; bmp1(); }
  delay(500);

  sx0=SCR_WD; sy0=SCR_HT/2; // right
  for(sc=32;sc>0;sc--) {  sx=sy=sc; bmp1(); }
  for(sc=0;sc<=32;sc++) { sx=sy=sc; bmp2(); }
  delay(500);

  sx0=SCR_WD/2; sy0=SCR_HT; // down
  for(sc=32;sc>0;sc--) {  sx=sy=sc; bmp2(); }
  for(sc=0;sc<=32;sc++) { sx=sy=sc; bmp1(); }
  delay(500);
}

