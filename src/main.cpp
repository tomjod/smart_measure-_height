#include <Arduino.h> // Arduino core library
#include <NewPing.h> // Ultrasonic sensor library
#include <Adafruit_GFX.h> // Core graphics library
#include <ST7789_AVR.h> // ST7789 library
#include "RREFont.h" 
#include "rre_digits_arial120v.h"
#include "rre_chicago_20x24.h"

int distance_from_floor = 200; // Distance from the floor in cm
int iterations = 10; // Number of iterations for median filter
int last_height_reading = 0; // Last distance reading
int tolerance = 1; // Tolerance for displaying height

#define TRIGGER_PIN  2 // Trigger pin for ultrasonic sensor
#define ECHO_PIN     3 // Echo pin for ultrasonic sensor
#define MAX_DISTANCE 200 // Maximum distance to measure
#define ITERATIONS  10  // Número de lecturas para el promedio
#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

// The other display pins (SDA and SCL) 
// are connected to Arduino hardware SPI pins (digital pin 11 and digital pin 13).
#define TFT_CS    10  // chip select pin
#define TFT_DC     9  // data/command pin
#define TFT_RST    8  // reset pin
#define SCR_WD 320
#define SCR_HT 170

unsigned long pingTimer[ITERATIONS]; // Holds the times when the next ping should happen for each iteration.
unsigned int cm[ITERATIONS];         // Where the ping distances are stored.
uint8_t currentIteration = 0;        // Keeps track of iteration step.

ST7789_AVR lcd = ST7789_AVR(TFT_DC, TFT_RST, TFT_CS);
RREFont font;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) { return lcd.fillRect(x, y, w, h, c); }

// Initialize NewPing sonar
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void display_reading(String text);
void display_welcome_message();



bool displayNeedsUpdate = true;  // Flag para controlar actualizaciones de pantalla
bool first_reading = true;

unsigned long lastChangeTime = 0;  // Tiempo de la última actualización de height

unsigned long displayTimer = 0;   // Timer para actualizaciones de pantalla
unsigned long welcomeTimer = 0;   // Timer para mensaje de bienvenida

const unsigned int DISPLAY_INTERVAL = 1000; // Intervalo de actualización de pantalla (ms)
const unsigned int WELCOME_DURATION = 4000; // Duración del mensaje de bienvenida (ms)

bool welcomeScreenActive = false;

#define INACTIVITY_TIMEOUT 60000  // 60 segundos de inactividad para volver a bienvenida

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  lcd.init();
  lcd.fillScreen(BLACK);
  lcd.setRotation(3);
  
  pingTimer[0] = millis() + 75;            // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < ITERATIONS; i++) // Set the starting time for each iteration.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  displayTimer = millis();
  
  display_welcome_message();
  welcomeScreenActive = true;
  welcomeTimer = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // Actualizar lectura del sensor
   for (uint8_t i = 0; i < ITERATIONS; i++) { // Loop through all the iterations.
    if (millis() >= pingTimer[i]) {          // Is it this iteration's time to ping?
      pingTimer[i] += PING_INTERVAL * ITERATIONS; // Set next time this sensor will be pinged.
      if (i == 0 && currentIteration == ITERATIONS - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar.timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentIteration = i;        // Sensor being accessed.
      cm[currentIteration] = 0;    // Make distance zero in case there's no ping echo for this iteration.
      sonar.ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
  
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar.check_timer())
    cm[currentIteration] = sonar.ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle() { // All iterations complete, calculate the median.
  unsigned int height = 0;
  
  unsigned int uS[ITERATIONS];
  uint8_t j, it = ITERATIONS;
  uS[0] = NO_ECHO;
  for (uint8_t i = 0; i < it; i++) { // Loop through iteration results.
    if (cm[i] != NO_ECHO) { // Ping in range, include as part of median.
      if (i > 0) {          // Don't start sort till second ping.
        for (j = i; j > 0 && uS[j - 1] < cm[i]; j--) // Insertion sort loop.
          uS[j] = uS[j - 1];                         // Shift ping array to correct position for sort insertion.
      } else j = 0;         // First ping is sort starting point.
      uS[j] = cm[i];        // Add last ping to array in sorted position.
    } else it--;            // Ping out of range, skip and don't include as part of median.
  }
  unsigned int filtered_distance = uS[it >> 1];
  Serial.print(filtered_distance);
  Serial.println("cm");

  // Calcular altura usando el promedio
  height = distance_from_floor - filtered_distance;
  int diff = height - last_height_reading;
  unsigned long currentMillis = millis();
         
  // Verificar timeout por inactividad
  if (currentMillis - lastChangeTime > INACTIVITY_TIMEOUT) {
    display_welcome_message();
    welcomeScreenActive = true;
    displayNeedsUpdate = true;
    last_height_reading = 0;  // Reset de la última lectura
    return;
  }
  
  // Actualizar pantalla si es necesario
  if (height != last_height_reading) {
    if (height >= 50 && height <= 200) {  // Rango válido de altura
      if (diff < -50) {  // Cambio brusco negativo (persona sale)
        display_out_of_range();
        last_height_reading = height;
        lastChangeTime = currentMillis;  // Actualizar tiempo de última actividad
      } else {  // Cambio normal o persona entrando
        display_reading(String(height));
        last_height_reading = height;
        lastChangeTime = currentMillis;  // Actualizar tiempo de última actividad
        displayNeedsUpdate = true;
      }
    } else {  // Fuera del rango válido de altura
      display_out_of_range();
    }
  }
}

void display_reading(String height) {
  lcd.fillScreen(BLACK);
  font.init(customRect, SCR_WD, SCR_HT);
  
  char* numbers = const_cast<char*>(height.c_str());

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
  
  delay(WELCOME_DURATION);
}

// Agregar esta función para mostrar el mensaje de fuera de rango
void display_out_of_range() {
  if (displayNeedsUpdate) {
    lcd.fillScreen(BLACK);
    font.init(customRect, SCR_WD, SCR_HT);
    font.setFont(&rre_chicago_20x24);
    font.setScale(1, 1);
    
    // Mensajes a mostrar
    char* msg1 = "Por favor";
    char* msg2 = "coloquese debajo";
    char* msg3 = "del sensor";
    
    // Calcular posiciones para centrar
    int16_t y = SCR_HT / 4;
    int16_t spacing = 35; // Espacio entre líneas
    
    // Color rojo suave para advertencia
    uint16_t warningColor = RGBto565(255, 100, 100);
    
    // Mostrar cada línea centrada
    font.setColor(warningColor);
    font.printStr((SCR_WD - font.strWidth(msg1)) / 2, y, msg1);
    font.printStr((SCR_WD - font.strWidth(msg2)) / 2, y + spacing, msg2);
    font.printStr((SCR_WD - font.strWidth(msg3)) / 2, y + spacing * 2, msg3);
    
    // Dibujar un ícono o símbolo de advertencia
    int iconSize = 20;
    int iconX = (SCR_WD - iconSize) / 2;
    int iconY = y + spacing * 3;
    
    // Dibujar un triángulo de advertencia simple
    for(int i = 0; i < iconSize; i++) {
      lcd.drawLine(iconX + i, iconY + iconSize - i/2,
                  iconX + i, iconY + iconSize - i/2,
                  warningColor);
    }
    delay(2000);
    displayNeedsUpdate = false;
  }
}
