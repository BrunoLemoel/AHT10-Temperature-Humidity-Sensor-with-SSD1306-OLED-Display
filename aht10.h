#ifndef AHT10_H
#define AHT10_H

#include <stdint.h>
#include <stdbool.h>

// Configuração do AHT10
#define AHT10_I2C_PORT i2c0
#define AHT10_SDA_PIN 0
#define AHT10_SCL_PIN 1
#define AHT10_I2C_FREQ 400000

// Endereço I2C do AHT10
#define AHT10_I2C_ADDR 0x38

// Comandos do AHT10
#define AHT10_CMD_INIT     0xE1    // Comando de inicialização
#define AHT10_CMD_TRIGGER  0xAC    // Comando para iniciar medição
#define AHT10_CMD_SOFTRST  0xBA    // Comando de reset
#define AHT10_CMD_STATUS   0x71    // Comando para ler status

// Parâmetros do comando de trigger
#define AHT10_TRIGGER_DATA1 0x33
#define AHT10_TRIGGER_DATA2 0x00

// Bits de status
#define AHT10_STATUS_BUSY     0x80  // Bit 7: 1 = ocupado
#define AHT10_STATUS_CALIBRATED 0x08  // Bit 3: 1 = calibrado

// Estrutura para dados do sensor
typedef struct {
    float temperature;
    float humidity;
    bool valid;
} aht10_data_t;

// Funções do sensor AHT10
bool aht10_init(void);
bool aht10_detect(void);
bool aht10_read_data(aht10_data_t* data);
bool aht10_trigger_measurement(void);
bool aht10_is_ready(void);
const char* aht10_get_comfort_level(float temp, float humidity);

#endif // AHT10_H
