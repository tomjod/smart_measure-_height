# Medidor de Altura Inteligente (Smart Height Measure)

## Descripción
Este proyecto es un medidor de altura inteligente que utiliza un sensor ultrasónico para medir la estatura de una persona y mostrarla en una pantalla TFT ST7789. El dispositivo está construido con Arduino y proporciona mediciones precisas en tiempo real con una interfaz visual atractiva.

## Características
- Medición precisa de altura utilizando sensor ultrasónico
- Pantalla TFT a color para visualización de resultados
- Filtro de mediana para lecturas más estables
- Interfaz visual amigable con animaciones y emojis
- Tolerancia configurable para evitar fluctuaciones
- Modo de espera con animación "QUANTUM" cuando no hay lecturas

## Componentes Necesarios
- Arduino (compatible con Arduino UNO)
- Sensor ultrasónico HC-SR04
- Pantalla TFT ST7789 (170x320 píxeles)
- Cables de conexión

## Conexiones
- **Sensor Ultrasónico:**
  - TRIGGER_PIN: Pin 3
  - ECHO_PIN: Pin 2

- **Pantalla TFT ST7789:**
  - CS: Pin 10
  - DC: Pin 9
  - RESET: Pin 8
  - SDA: Pin 11 (MOSI)
  - SCL: Pin 13 (SCK)

## Bibliotecas Requeridas
- NewPing
- Adafruit_ST7789
- Adafruit_GFX
- WiFiEsp



## Configuración
El dispositivo puede ser configurado ajustando las siguientes variables:
- `distance_from_floor`: Distancia desde el piso en cm (por defecto 200 cm)
- `iterations`: Número de iteraciones para el filtro de mediana (por defecto 5)
- `tolerance`: Tolerancia para evitar fluctuaciones (por defecto 1 cm)

## Instrucciones de Uso
1. Conecta el sensor ultrasónico al Arduino según las conexiones indicadas.
2. Conecta la pantalla TFT ST7789 al Arduino según las conexiones indicadas.
3. Carga el código en el Arduino.
4. Ajusta las variables de configuración según tus necesidades.
5. Enciende el Arduino y observa la altura medida en la pantalla TFT.

Autor: [Alexis Ocando](https://github.com/tomjod)
