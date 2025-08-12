#include "display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ===== CONFIGURAÇÕES =====
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_FREQ 400000
#define SSD1306_ADDR 0x3C

// Configurações específicas para SSD1306 128x64
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64
#define SSD1306_PAGES  (SSD1306_HEIGHT / 8)

// ===== COMANDOS SSD1306 =====
#define SSD1306_SETCONTRAST          0x81
#define SSD1306_DISPLAYALLON_RESUME  0xA4
#define SSD1306_DISPLAYALLON         0xA5
#define SSD1306_NORMALDISPLAY        0xA6
#define SSD1306_INVERTDISPLAY        0xA7
#define SSD1306_DISPLAYOFF           0xAE
#define SSD1306_DISPLAYON            0xAF
#define SSD1306_SETDISPLAYOFFSET     0xD3
#define SSD1306_SETCOMPINS           0xDA
#define SSD1306_SETVCOMDETECT        0xDB
#define SSD1306_SETDISPLAYCLOCKDIV   0xD5
#define SSD1306_SETPRECHARGE         0xD9
#define SSD1306_SETMULTIPLEX         0xA8
#define SSD1306_SETLOWCOLUMN         0x00
#define SSD1306_SETHIGHCOLUMN        0x10
#define SSD1306_SETSTARTLINE         0x40
#define SSD1306_MEMORYMODE           0x20
#define SSD1306_COLUMNADDR           0x21
#define SSD1306_PAGEADDR             0x22
#define SSD1306_COMSCANINC           0xC0
#define SSD1306_COMSCANDEC           0xC8
#define SSD1306_SEGREMAP             0xA0
#define SSD1306_CHARGEPUMP           0x8D
#define SSD1306_EXTERNALVCC          0x1
#define SSD1306_SWITCHCAPVCC         0x2

// ===== VARIÁVEIS GLOBAIS =====
static bool display_initialized = false;
static uint8_t display_buffer[SSD1306_WIDTH * SSD1306_PAGES];

// ===== FUNÇÕES AUXILIARES =====

bool ssd1306_send_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    int result = i2c_write_blocking(I2C_PORT, SSD1306_ADDR, buf, 2, false);
    return result == 2;
}

bool ssd1306_send_data(const uint8_t *data, size_t len) {
    uint8_t buf[len + 1];
    buf[0] = 0x40;  // Data mode
    memcpy(buf + 1, data, len);
    int result = i2c_write_blocking(I2C_PORT, SSD1306_ADDR, buf, len + 1, false);
    return result == (len + 1);
}

// Fonte bitmap simples 5x8 para caracteres essenciais
static const uint8_t font_5x8[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0' - 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1' - 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2' - 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3' - 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4' - 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5' - 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6' - 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7' - 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8' - 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9' - 9
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A' - 10
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B' - 11
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C' - 12
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D' - 13
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E' - 14
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F' - 15
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G' - 16
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H' - 17
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I' - 18
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J' - 19
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K' - 20
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L' - 21
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M' - 22
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N' - 23
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O' - 24
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P' - 25
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q' - 26
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R' - 27
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S' - 28
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T' - 29
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U' - 30
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V' - 31
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 'W' - 32
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X' - 33
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y' - 34
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z' - 35
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' - 36 (espaço)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!' - 37
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':' - 38
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '-' - 39
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.' - 40
    {0x08, 0x1C, 0x2A, 0x08, 0x08}, // '%' - 41
};

// Função para obter índice da fonte
uint8_t get_font_index(char c) {
    if (c >= '0' && c <= '9') return c - '0';           // 0-9 (índices 0-9)
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;     // A-Z (índices 10-35)
    if (c == ' ') return 36;  // espaço
    if (c == '!') return 37;  // exclamação
    if (c == ':') return 38;  // dois pontos
    if (c == '-') return 39;  // hífen
    if (c == '.') return 40;  // ponto
    if (c == '%') return 41;  // porcentagem
    return 36; // Espaço como padrão
}

// ===== FUNÇÕES PRINCIPAIS =====

bool display_init(void) {
    printf("[DISPLAY] Inicializando I2C e SSD1306...\n");
    
    // Configurar I2C
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    sleep_ms(100);
    
    // Verificar se o dispositivo responde
    uint8_t test_data = 0x00;
    int result = i2c_write_blocking(I2C_PORT, SSD1306_ADDR, &test_data, 1, false);
    if (result < 0) {
        printf("[DISPLAY] Erro: SSD1306 não detectado no endereço 0x%02X\n", SSD1306_ADDR);
        return false;
    }
    
    printf("[DISPLAY] SSD1306 detectado! Configurando...\n");
    
    // Sequência de inicialização para 128x64
    ssd1306_send_command(SSD1306_DISPLAYOFF);
    ssd1306_send_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_send_command(0x80);
    ssd1306_send_command(SSD1306_SETMULTIPLEX);
    ssd1306_send_command(63);  // 64 lines
    ssd1306_send_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_send_command(0x0);
    ssd1306_send_command(SSD1306_SETSTARTLINE | 0x0);
    ssd1306_send_command(SSD1306_CHARGEPUMP);
    ssd1306_send_command(0x14);
    ssd1306_send_command(SSD1306_MEMORYMODE);
    ssd1306_send_command(0x00);
    ssd1306_send_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_send_command(SSD1306_COMSCANDEC);
    ssd1306_send_command(SSD1306_SETCOMPINS);
    ssd1306_send_command(0x12);  // Alternative COM config
    ssd1306_send_command(SSD1306_SETCONTRAST);
    ssd1306_send_command(0xCF);
    ssd1306_send_command(SSD1306_SETPRECHARGE);
    ssd1306_send_command(0xF1);
    ssd1306_send_command(SSD1306_SETVCOMDETECT);
    ssd1306_send_command(0x40);
    ssd1306_send_command(SSD1306_DISPLAYALLON_RESUME);
    ssd1306_send_command(SSD1306_NORMALDISPLAY);
    ssd1306_send_command(SSD1306_DISPLAYON);
    
    display_initialized = true;
    printf("[DISPLAY] ✅ SSD1306 128x64 inicializado com sucesso!\n");
    
    // Limpar o buffer
    memset(display_buffer, 0, sizeof(display_buffer));
    
    return true;
}

void display_clear(uint16_t color) {
    if (!display_initialized) return;
    
    uint8_t pattern = (color == COLOR_BLACK) ? 0x00 : 0xFF;
    
    // Configurar área completa
    ssd1306_send_command(SSD1306_COLUMNADDR);
    ssd1306_send_command(0);   // Start column
    ssd1306_send_command(SSD1306_WIDTH - 1);   // End column
    
    ssd1306_send_command(SSD1306_PAGEADDR);
    ssd1306_send_command(0);   // Start page
    ssd1306_send_command(SSD1306_PAGES - 1);   // End page
    
    // Limpar toda a tela
    uint8_t data[128];  // Uma linha por vez
    memset(data, pattern, sizeof(data));
    
    for (int page = 0; page < SSD1306_PAGES; page++) {
        ssd1306_send_data(data, sizeof(data));
    }
}

void display_print_text_bitmap(uint8_t x, uint8_t y, const char* text, bool invert) {
    if (!display_initialized || !text) return;
    
    // Verificar limites
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    
    uint8_t page = y / 8;
    if (page >= SSD1306_PAGES) return;
    
    printf("[DISPLAY] Bitmap (%d,%d): '%s' %s\n", x, y, text, invert ? "(invertido)" : "");
    
    // Configurar posição
    ssd1306_send_command(SSD1306_COLUMNADDR);
    ssd1306_send_command(x);
    ssd1306_send_command(SSD1306_WIDTH - 1);
    
    ssd1306_send_command(SSD1306_PAGEADDR);
    ssd1306_send_command(page);
    ssd1306_send_command(page);
    
    // Renderizar cada caractere
    for (int i = 0; i < strlen(text) && (x + i * 6) < SSD1306_WIDTH; i++) {
        char c = text[i];
        uint8_t font_idx = get_font_index(c);
        
        // Enviar os 5 pixels da fonte + 1 pixel de espaçamento
        uint8_t char_data[6];
        for (int j = 0; j < 5; j++) {
            char_data[j] = invert ? ~font_5x8[font_idx][j] : font_5x8[font_idx][j];
        }
        char_data[5] = invert ? 0xFF : 0x00;  // Espaçamento
        
        ssd1306_send_data(char_data, 6);
    }
}

// Função para compatibilidade (mantida simples)
void display_print_text(uint16_t x, uint16_t y, const char* text, uint16_t color, uint16_t bg_color) {
    display_print_text_bitmap((uint8_t)x, (uint8_t)y, text, color == COLOR_BLACK);
}

// Atualizar dados do sensor no display - AJUSTADO PARA 128x64 COM ALERTAS
void display_update_sensor_data(aht10_data_t data) {
    if (!display_initialized) {
        printf("[DISPLAY OFFLINE] Temp: %.1f°C | Umidade: %.1f%%\n", 
               data.temperature, data.humidity);
        return;
    }
    
    if (!data.valid) {
        display_show_error_screen("Erro de leitura do sensor");
        return;
    }
    
    // Compensação de temperatura (ajuste baseado na diferença observada)
    float temp_compensada = data.temperature - 2.3f;  // Corrige diferença com estações meteorológicas
    
    char temp_str[32];
    char humidity_str[32];
    char status_str[32];
    
    // Preparar strings para 128x64 COM ALERTAS (usando temperatura compensada)
    // Temperatura com alertas condicionais
    if (temp_compensada < 20.0f) {
        snprintf(temp_str, sizeof(temp_str), "%.1fC  %s", temp_compensada, ALERT_LOW_TEMP);
    } else if (temp_compensada > 40.0f) {
        snprintf(temp_str, sizeof(temp_str), "%.1fC  %s", temp_compensada, ALERT_HIGH_TEMP);
    } else {
        snprintf(temp_str, sizeof(temp_str), "%.1fC", temp_compensada);
    }
    
    // Umidade com alerta condicional
    if (data.humidity > 70.0f) {
        snprintf(humidity_str, sizeof(humidity_str), "%.1f%%  %s", data.humidity, ALERT_HIGH_HUMIDITY);
    } else {
        snprintf(humidity_str, sizeof(humidity_str), "%.1f%%", data.humidity);
    }
    
    // Determinar status baseado na temperatura compensada e umidade
    if (temp_compensada < 18.0f) {
        strcpy(status_str, "FRIO");
    } else if (temp_compensada > 28.0f) {
        strcpy(status_str, "QUENTE");
    } else if (data.humidity < 40.0f) {
        strcpy(status_str, "SECO");
    } else if (data.humidity > 70.0f) {
        strcpy(status_str, "UMIDO");
    } else {
        strcpy(status_str, "IDEAL");
    }
    
    // Limpar tela
    display_clear(COLOR_BLACK);
    
    // Layout otimizado para 128x64 usando fonte bitmap
    display_print_text_bitmap(0, 0, "AHT10", false);      // Título na linha 0
    display_print_text_bitmap(0, 16, temp_str, false);    // Temperatura na linha 16 (com alerta se necessário)
    display_print_text_bitmap(0, 32, humidity_str, false); // Umidade na linha 32 (com alerta se necessário)
    display_print_text_bitmap(0, 48, status_str, false);   // Status na linha 48
}

// Tela de inicialização - AJUSTADA PARA 128x64
void display_show_startup_screen(void) {
    if (!display_initialized) return;
    
    display_clear(COLOR_BLACK);
    
    // Layout compacto para 128x64
    display_print_text_bitmap(0, 0, "SENSOR AHT10", false);
    display_print_text_bitmap(0, 16, "INICIANDO...", false);
    display_print_text_bitmap(0, 48, "AGUARDE", false);
}

// Tela de erro - AJUSTADA PARA 128x64
void display_show_error_screen(const char* error_msg) {
    if (!display_initialized) return;
    
    display_clear(COLOR_BLACK);
    
    // Layout de erro para 128x64
    display_print_text_bitmap(0, 0, "ERRO!", true);  // Invertido para destacar
    
    // Quebrar mensagem de erro se necessário
    if (strlen(error_msg) <= 21) {  // 21 chars cabem em 128 pixels (21*6=126)
        display_print_text_bitmap(0, 24, error_msg, false);
    } else {
        // Mostrar só uma parte da mensagem
        char short_msg[22];
        strncpy(short_msg, error_msg, 21);
        short_msg[21] = '\0';
        display_print_text_bitmap(0, 24, short_msg, false);
    }
    
    display_print_text_bitmap(0, 48, "VERIFIQUE CONEXAO", false);
}

bool display_is_ready(void) {
    return display_initialized;
}
