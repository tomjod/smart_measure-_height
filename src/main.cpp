#include <Arduino.h> // Arduino core library
#include <NewPing.h> // Ultrasonic sensor library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_GFX.h> // Core graphics library
#include <WiFiEsp.h> // WiFi library

int distance_from_floor = 200; // Distance from the floor in cm
int iterations = 5; // Number of iterations for median filter
int last_distance = 0; // Last distance reading
int tolerance = 1; // Tolerance for displaying height

#define TRIGGER_PIN  3 // Trigger pin for ultrasonic sensor
#define ECHO_PIN     2 // Echo pin for ultrasonic sensor
#define MAX_DISTANCE 200 // Maximum distance to measure
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RES    8  // define reset pin
// The other display pins (SDA and SCL) 
// are connected to Arduino hardware SPI pins (digital pin 11 and digital pin 13).


// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RES);

// Initialize NewPing sonar
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// put function declarations here:
void draw_smiley(int x, int y, int size);
void display_text(String text);
void display_no_readings();

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  // if the display has CS pin try with SPI_MODE0
  tft.init(170, 320, SPI_MODE3);    // Init ST7789 display 170x320 pix

  tft.setRotation(2);
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
    tft.fillCircle(x, y, size, ST77XX_YELLOW);
    
    // Ojos
    int eye_size = size/4;
    tft.fillCircle(x - size/3, y - size/4, eye_size, ST77XX_BLACK);
    tft.fillCircle(x + size/3, y - size/4, eye_size, ST77XX_BLACK);
    
    // Sonrisa
    int smile_y = y + size/4;
    tft.fillCircle(x, smile_y, size/3, ST77XX_BLACK);
    tft.fillCircle(x, smile_y - size/6, size/2, ST77XX_YELLOW);
}


void display_text(String text) {
  tft.fillScreen(ST77XX_BLACK);
  
  // Centrar el texto horizontalmente
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(3);
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (tft.width() - w) / 2;
  
  // Posicionar verticalmente en el tercio superior
  int y = tft.height() / 3;
  
  // Dibujar un rectángulo redondeado como fondo
  tft.fillRoundRect(x-10, y-10, w+20, h+20, 10, ST77XX_BLUE);
  
  // Mostrar el texto
  tft.setCursor(x, y);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(text);
  
  // Dibujar línea decorativa debajo
  tft.drawFastHLine(x-20, y+h+20, w+40, ST77XX_CYAN);
  draw_smiley(x, y+h+20, 20);
}


void display_no_readings() {
  tft.fillScreen(ST77XX_BLACK);
  
  // Dibujar "QUANTUM" en el centro
  tft.setTextSize(3);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds("QUANTUM", 0, 0, &x1, &y1, &w, &h);
  int x = (tft.width() - w) / 2;
  int y = (tft.height() - h) / 2;

  // Efecto de brillo/resplandor
  for(int i = 3; i >= 0; i--) {
    uint16_t color;
    switch(i) {
      case 3: color = ST77XX_BLUE; break;    // Exterior
      case 2: color = ST77XX_CYAN; break;    // Medio
      case 1: color = ST77XX_WHITE; break;   // Interior
      case 0: color = ST77XX_CYAN; break;    // Texto
    }
    tft.fillRoundRect(x-10-(i*5), y-10-(i*5), w+20+(i*10), h+20+(i*10), 10, color);
  }

  // Texto "QUANTUM"
  tft.setCursor(x, y);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("QUANTUM");

  // Líneas decorativas
  int lineLength = w + 40;
  int lineX = (tft.width() - lineLength) / 2;
  tft.drawFastHLine(lineX, y-20, lineLength, ST77XX_CYAN);
  tft.drawFastHLine(lineX, y+h+20, lineLength, ST77XX_CYAN);

  // Pequeño texto descriptivo
  tft.setTextSize(1);
  tft.setCursor((tft.width() - 11*6)/2, y+h+30);
  tft.print("Sin lecturas");
}
