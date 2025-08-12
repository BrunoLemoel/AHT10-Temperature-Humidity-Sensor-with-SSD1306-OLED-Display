#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "aht10.h"  // Incluir para usar aht10_data_t

// Configuração do display SSD1306 128x64
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64
#define DISPLAY_SDA_PIN 14
#define DISPLAY_SCL_PIN 15

// Cores para display monocromático SSD1306
#define COLOR_BLACK   0x00
#define COLOR_WHITE   0x01

// Símbolos de alerta
#define ALERT_HIGH_HUMIDITY   "!H"  // Umidade > 70%
#define ALERT_LOW_TEMP        "!C"  // Temperatura < 20°C  
#define ALERT_HIGH_TEMP       "!Q"  // Temperatura > 40°C

// Funções do display
bool display_init(void);
void display_clear(uint16_t color);
void display_update_sensor_data(aht10_data_t data);
void display_show_startup_screen(void);
void display_show_error_screen(const char* error_msg);

#endif // DISPLAY_H
