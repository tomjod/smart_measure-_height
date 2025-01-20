#include <Arduino.h> // Arduino core library
#include <NewPing.h> // Ultrasonic sensor library
#include <Adafruit_GFX.h> // Core graphics library
#include <ST7789_AVR.h> // ST7789 library
#include <SPI.h>


int distance_from_floor = 200; // Distance from the floor in cm
int iterations = 5; // Number of iterations for median filter
int last_distance = 0; // Last distance reading
int tolerance = 1; // Tolerance for displaying height

#define TRIGGER_PIN  3 // Trigger pin for ultrasonic sensor
#define ECHO_PIN     2 // Echo pin for ultrasonic sensor
#define MAX_DISTANCE 200 // Maximum distance to measure

// The other display pins (SDA and SCL) 
// are connected to Arduino hardware SPI pins (digital pin 11 and digital pin 13).
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin
#define SCR_WD 170
#define SCR_HT 320
ST7789_AVR lcd = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);

#include "RREFont.h"
#include "rre_chicago_20x24.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

// Initialize NewPing sonar
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// put function declarations here:
void draw_smiley(int x, int y, int size);
void display_text(String text);
void display_no_readings();

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
lcd.init();
  lcd.fillScreen(BLACK);

  font.init(customRect, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  font.setFont(&rre_chicago_20x24);
  int i;
  for(i=0;i<10;i++) {
    font.setColor(RGBto565(i*25,i*25,i*25));
    font.printStr(30+i,20+i,"Hola");
  }
  font.setColor(WHITE);
  font.printStr(30+i,20+i,"Hola");

  for(i=0;i<10;i++) {
    font.setColor(lcd.rgbWheel(0+i*8));
    font.printStr(25+i,60+i,"Quantum");
  }
  font.setColor(WHITE);
  font.printStr(25+i,60+i,"Quantum");
  delay(4000);
  lcd.fillScreen();
}

void loop() {
  int distance = (sonar.ping_median(iterations) / 2) * 0.343;
  int height = distance_from_floor - distance;

  if (height > tolerance && distance != last_distance) {
    String text = "Estatura: " + String(height) + " cm";
    display_text(text);
    Serial.println(text);
    last_distance = distance;
  } else if (distance <= tolerance) {
    display_no_readings();
  }

  delay(100);

}

void draw_smiley(int x, int y, int size) {
    // Cara - círculo amarillo
    lcd.fillCircle(x, y, size, YELLOW);
    
    // Ojos
    int eye_size = size/4;
    lcd.fillCircle(x - size/3, y - size/4, eye_size, BLACK);
    lcd.fillCircle(x + size/3, y - size/4, eye_size, BLACK);
    
    // Sonrisa
    int smile_y = y + size/4;
    lcd.fillCircle(x, smile_y, size/3, BLACK);
    lcd.fillCircle(x, smile_y - size/6, size/2, YELLOW);
}


void display_text(String text) {
  lcd.fillScreen(BLACK);
  
  // Centrar el texto horizontalmente
  int16_t x1, y1;
  uint16_t w, h;
  lcd.setTextSize(3);
  lcd.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (lcd.width() - w) / 2;
  
  // Posicionar verticalmente en el tercio superior
  int y = lcd.height() / 3;
  
  // Dibujar un rectángulo redondeado como fondo
  lcd.fillRoundRect(x-10, y-10, w+20, h+20, 10, BLUE);
  
  // Mostrar el texto
  lcd.setCursor(x, y);
  lcd.setTextColor(WHITE);
  lcd.print(text);
  
  // Dibujar línea decorativa debajo
  lcd.drawFastHLine(x-20, y+h+20, w+40, CYAN);
  draw_smiley(x, y+h+20, 20);
}


void display_no_readings() {
  lcd.fillScreen(BLACK);
  
  // Dibujar "QUANTUM" en el centro
  lcd.setTextSize(3);
  int16_t x1, y1;
  uint16_t w, h;
  lcd.getTextBounds("QUANTUM", 0, 0, &x1, &y1, &w, &h);
  int x = (lcd.width() - w) / 2;
  int y = (lcd.height() - h) / 2;

  // Efecto de brillo/resplandor
  for(int i = 3; i >= 0; i--) {
    uint16_t color;
    switch(i) {
      case 3: color = BLUE; break;    // Exterior
      case 2: color = CYAN; break;    // Medio
      case 1: color = WHITE; break;   // Interior
      case 0: color = CYAN; break;    // Texto
    }
    lcd.fillRoundRect(x-10-(i*5), y-10-(i*5), w+20+(i*10), h+20+(i*10), 10, color);
  }

  // Texto "QUANTUM"
  lcd.setCursor(x, y);
  lcd.setTextColor(WHITE);
  lcd.print("QUANTUM");

  // Líneas decorativas
  int lineLength = w + 40;
  int lineX = (lcd.width() - lineLength) / 2;
  lcd.drawFastHLine(lineX, y-20, lineLength, CYAN);
  lcd.drawFastHLine(lineX, y+h+20, lineLength, CYAN);

  // Pequeño texto descriptivo
  lcd.setTextSize(1);
  lcd.setCursor((lcd.width() - 11*6)/2, y+h+30);
  lcd.print("Sin lecturas");
}
