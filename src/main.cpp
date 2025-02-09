#include <Arduino.h> // Arduino core library
#include <NewPing.h> // Ultrasonic sensor library
#include <Adafruit_GFX.h> // Core graphics library
#include <ST7789_AVR.h> // ST7789 library


int distance_from_floor = 210; // Distance from the floor in cm
int iterations = 10; // Number of iterations for median filter
int last_height_reading = 0; // Last distance reading
int tolerance = 2; // Tolerance for displaying height

#define TRIGGER_PIN  2 // Trigger pin for ultrasonic sensor
#define ECHO_PIN     3 // Echo pin for ultrasonic sensor
#define MAX_DISTANCE 210 // Maximum distance to measure

// The other display pins (SDA and SCL) 
// are connected to Arduino hardware SPI pins (digital pin 11 and digital pin 13).
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin
#define SCR_WD 320
#define SCR_HT 170
ST7789_AVR lcd = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);

#include "RREFont.h"
#include "rre_digits_arial120v.h"
#include "rre_chicago_20x24.h"


RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

// Initialize NewPing sonar
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// put function declarations here:
void draw_smiley(int x, int y, int size);
void display_reading(String text);
void display_no_readings();
void display_welcome_message();

#define READINGS_COUNT 10  // Número de lecturas para el promedio
int readings[READINGS_COUNT];  // Array para almacenar lecturas
int readIndex = 0;  // Índice para las lecturas
int total = 0;  // Total para calcular el promedio
bool displayNeedsUpdate = true;  // Flag para controlar actualizaciones de pantalla

// Agregar estas variables globales al inicio
unsigned long lastChangeTime = 0;  // Tiempo de la última actualización de height
#define RESET_TIMEOUT 120000  // 120 segundos en milisegundos

// Agregar estas variables globales para el control de tiempo
unsigned long pingTimer = 0;     // Timer para lecturas del sensor
unsigned long displayTimer = 0;   // Timer para actualizaciones de pantalla
unsigned long welcomeTimer = 0;   // Timer para mensaje de bienvenida

const unsigned int PING_INTERVAL = 50;    // Intervalo entre lecturas (ms)
const unsigned int DISPLAY_INTERVAL = 1000; // Intervalo de actualización de pantalla (ms)
const unsigned int WELCOME_DURATION = 4000; // Duración del mensaje de bienvenida (ms)

bool welcomeScreenActive = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  lcd.init();
  lcd.fillScreen(BLACK);
  lcd.setRotation(3);
  
  // Inicializar array de lecturas
  for (int i = 0; i < READINGS_COUNT; i++) {
    readings[i] = 0;
  }
  
  // Inicializar timers
  pingTimer = millis();
  displayTimer = millis();
  
  display_welcome_message();
  welcomeScreenActive = true;
  welcomeTimer = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Manejar el mensaje de bienvenida
  if (welcomeScreenActive && currentMillis - welcomeTimer >= WELCOME_DURATION) {
    welcomeScreenActive = false;
    displayNeedsUpdate = true;
  }
  
  // Actualizar lectura del sensor
  if (currentMillis >= pingTimer) {
    pingTimer += PING_INTERVAL;
    
    // Obtener nueva lectura
    int new_reading = sonar.ping_cm();
    
    // Actualizar el promedio móvil
    total = total - readings[readIndex];
    readings[readIndex] = new_reading;
    total = total + readings[readIndex];
    readIndex = (readIndex + 1) % READINGS_COUNT;
  }
  
  // Actualizar pantalla
  if (currentMillis >= displayTimer && !welcomeScreenActive) {
    displayTimer += DISPLAY_INTERVAL;
    
    // Calcular altura usando el promedio
    int filtered_distance = total / READINGS_COUNT;
    int height = distance_from_floor - filtered_distance;
    
    // Verificar timeout de inactividad
    if (currentMillis - lastChangeTime > RESET_TIMEOUT && last_height_reading != 0) {
      last_height_reading = 0;
      displayNeedsUpdate = true;
      display_welcome_message();
      welcomeScreenActive = true;
      welcomeTimer = currentMillis;
      return;
    }
    
    // Actualizar pantalla si es necesario
    if (height > tolerance && abs(height - last_height_reading) > tolerance) {
      if (height >= 50 && height <= 250) {
        String text = "Estatura: " + String(height) + " cm";
        display_reading(text);
        last_height_reading = height;
        lastChangeTime = currentMillis;
        displayNeedsUpdate = false;
      }
    } else if (filtered_distance <= tolerance && displayNeedsUpdate) {
      display_no_readings();
      displayNeedsUpdate = false;
    }
    
    // Preparar siguiente actualización si hay cambios significativos
    if (abs(height - last_height_reading) > tolerance) {
      displayNeedsUpdate = true;
      lastChangeTime = currentMillis;
    }
  }
}

void display_reading(String text) {
  lcd.fillScreen(BLACK);
  font.init(customRect, SCR_WD, SCR_HT);
  
  // Extraer solo los números
  String numberPart = text;
  numberPart.replace("Estatura: ", "");
  numberPart.replace(" cm", "");
  char* numbers = const_cast<char*>(numberPart.c_str());
  
  // Configurar fuente grande para números
  font.setFont(&rre_digits_arial120v);
  int16_t numWidth = font.strWidth(numbers);
  int16_t numHeight = font.getHeight();
  
  // Configurar fuente pequeña para "cm"
  font.setFont(&rre_chicago_20x24);
  font.setScale(2, 2);
  String units = "cm";
  char* msg2 = const_cast<char*>(units.c_str());
  int16_t unitsWidth = font.strWidth(msg2);
  
  // Calcular posición total centrada
  int totalWidth = numWidth + unitsWidth + 20; // Más espacio entre número y unidad
  int numX = (SCR_WD - totalWidth) / 2;
  int numY = (SCR_HT - numHeight) / 2 - 10; // Subir un poco todo el conjunto
  
  // Mostrar números
  font.setFont(&rre_digits_arial120v);
  font.setScale(1, 1);
  font.setColor(RGBto565(77, 198, 162)); // Color turquesa Quantum
  font.printStr(numX, numY, numbers);
  
  // Mostrar "cm" a la derecha del número
  font.setFont(&rre_chicago_20x24);
  font.setScale(2, 2);
  font.setColor(WHITE); // Color turquesa Quantum
  font.printStr(numX + numWidth + 20, numY + numHeight/2 - font.getHeight()/2 + 30, msg2);
  
  // Línea decorativa sutil
  int lineY = numY + numHeight + 20;
  int lineWidth = totalWidth + 60;
  int lineX = (SCR_WD - lineWidth) / 2;
  lcd.drawFastHLine(lineX, lineY, lineWidth, RGBto565(40, 40, 40)); // Gris oscuro sutil
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



void display_welcome_message() {
  font.init(customRect, SCR_WD, SCR_HT);
  font.setFont(&rre_chicago_20x24);
  
  // Calculate width for both texts
  int16_t w1 = font.strWidth("Telemedicina");
  int16_t w2 = font.strWidth("Quantum");
  
  // Center horizontally
  int x1 = (SCR_WD - w1) / 2;
  int x2 = (SCR_WD - w2) / 2;
  
  // Position vertically
  int y = SCR_HT / 3;

  // Color turquesa del logo Quantum
  uint16_t quantumColor = RGBto565(77, 198, 162);  // #4DC6A2 in hex
  uint16_t quantumColor2 = RGBto565(255, 255, 255);  // #4DC6A2 in hex

  
  int i;
  font.setColor(quantumColor2);
  font.printStr(x1+i,y+i,"Telemedicina");
  

  y = y + 40;
  
  font.setColor(quantumColor);
  font.printStr(x2+i,y+i, "Quantum");
  

  delay(4000);
}
