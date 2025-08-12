#include "aht10.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// Variáveis globais
static bool aht10_initialized = false;

// Inicialização do I2C para AHT10
bool aht10_init(void) {
    printf("Inicializando sensor AHT10...\n");
    printf("  - I2C: I2C0\n");
    printf("  - SDA: GPIO %d\n", AHT10_SDA_PIN);
    printf("  - SCL: GPIO %d\n", AHT10_SCL_PIN);
    printf("  - Frequência: %d Hz\n", AHT10_I2C_FREQ);
    
    // Inicializar I2C
    i2c_init(AHT10_I2C_PORT, AHT10_I2C_FREQ);
    
    // Configurar pinos
    gpio_set_function(AHT10_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(AHT10_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(AHT10_SDA_PIN);
    gpio_pull_up(AHT10_SCL_PIN);
    
    printf("I2C inicializado com sucesso!\n");
    
    // Aguardar estabilização
    sleep_ms(100);
    
    // Detectar sensor
    if (!aht10_detect()) {
        printf("❌ AHT10 não detectado!\n");
        return false;
    }
    
    // Reset do sensor
    printf("Executando reset do AHT10...\n");
    uint8_t reset_cmd = AHT10_CMD_SOFTRST;
    int ret = i2c_write_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &reset_cmd, 1, false);
    if (ret < 0) {
        printf("❌ Falha no reset do AHT10\n");
        return false;
    }
    sleep_ms(20);  // Aguardar reset
    
    // Comando de inicialização/calibração
    printf("Inicializando/calibrando AHT10...\n");
    uint8_t init_cmd[3] = {AHT10_CMD_INIT, 0x08, 0x00};
    ret = i2c_write_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, init_cmd, 3, false);
    if (ret < 0) {
        printf("❌ Falha na inicialização do AHT10\n");
        return false;
    }
    sleep_ms(300);  // Aguardar calibração
    
    // Verificar status de calibração
    uint8_t status;
    ret = i2c_write_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &(uint8_t){AHT10_CMD_STATUS}, 1, true);
    if (ret >= 0) {
        ret = i2c_read_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &status, 1, false);
        if (ret >= 0 && (status & AHT10_STATUS_CALIBRATED)) {
            printf("✅ AHT10 calibrado com sucesso!\n");
        } else {
            printf("⚠️ AHT10 pode não estar calibrado corretamente\n");
        }
    }
    
    aht10_initialized = true;
    printf("✅ AHT10 inicializado e pronto!\n");
    return true;
}

// Detectar se o AHT10 está presente
bool aht10_detect(void) {
    printf("\n--- DETECÇÃO DO SENSOR AHT10 ---\n");
    printf("Testando endereço 0x%02X...\n", AHT10_I2C_ADDR);
    
    uint8_t status;
    int ret = i2c_write_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &(uint8_t){AHT10_CMD_STATUS}, 1, true);
    if (ret >= 0) {
        ret = i2c_read_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &status, 1, false);
        if (ret >= 0) {
            printf("✅ AHT10 encontrado no endereço 0x%02X\n", AHT10_I2C_ADDR);
            printf("Status inicial: 0x%02X\n", status);
            return true;
        }
    }
    
    printf("❌ AHT10 não encontrado\n");
    printf("Verifique:\n");
    printf("  - Conexões físicas (SDA=GPIO%d, SCL=GPIO%d)\n", AHT10_SDA_PIN, AHT10_SCL_PIN);
    printf("  - Alimentação do sensor (3.3V)\n");
    printf("  - Resistores pull-up nas linhas I2C\n");
    
    return false;
}

// Verificar se o sensor está pronto para leitura
bool aht10_is_ready(void) {
    if (!aht10_initialized) return false;
    
    uint8_t status;
    int ret = i2c_write_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &(uint8_t){AHT10_CMD_STATUS}, 1, true);
    if (ret < 0) return false;
    
    ret = i2c_read_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, &status, 1, false);
    if (ret < 0) return false;
    
    // Sensor está pronto quando não está ocupado
    return !(status & AHT10_STATUS_BUSY);
}

// Iniciar medição
bool aht10_trigger_measurement(void) {
    if (!aht10_initialized) return false;
    
    uint8_t trigger_cmd[3] = {AHT10_CMD_TRIGGER, AHT10_TRIGGER_DATA1, AHT10_TRIGGER_DATA2};
    int ret = i2c_write_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, trigger_cmd, 3, false);
    
    return ret >= 0;
}

// Ler dados do sensor
bool aht10_read_data(aht10_data_t* data) {
    if (!aht10_initialized || !data) {
        if (data) data->valid = false;
        return false;
    }
    
    // Iniciar medição
    if (!aht10_trigger_measurement()) {
        printf("❌ Falha ao iniciar medição\n");
        data->valid = false;
        return false;
    }
    
    // Aguardar medição (típico: 75ms)
    sleep_ms(80);
    
    // Aguardar sensor ficar pronto
    int timeout = 100; // 100ms timeout
    while (!aht10_is_ready() && timeout > 0) {
        sleep_ms(1);
        timeout--;
    }
    
    if (timeout <= 0) {
        printf("❌ Timeout aguardando sensor\n");
        data->valid = false;
        return false;
    }
    
    // Ler 6 bytes de dados
    uint8_t raw_data[6];
    int ret = i2c_read_blocking(AHT10_I2C_PORT, AHT10_I2C_ADDR, raw_data, 6, false);
    
    if (ret < 0) {
        printf("❌ Falha na leitura dos dados\n");
        data->valid = false;
        return false;
    }
    
    // Verificar se os dados são válidos (status byte)
    if (raw_data[0] & AHT10_STATUS_BUSY) {
        printf("⚠️ Sensor ainda ocupado\n");
        data->valid = false;
        return false;
    }
    
    // Converter dados brutos para valores físicos
    // Umidade: bits 19:0 dos bytes 1-3
    uint32_t humidity_raw = ((uint32_t)raw_data[1] << 12) | 
                           ((uint32_t)raw_data[2] << 4) | 
                           ((uint32_t)raw_data[3] >> 4);
    
    // Temperatura: bits 19:0 dos bytes 3-5
    uint32_t temperature_raw = (((uint32_t)raw_data[3] & 0x0F) << 16) | 
                              ((uint32_t)raw_data[4] << 8) | 
                              ((uint32_t)raw_data[5]);
    
    // Converter para valores reais
    data->humidity = (float)humidity_raw * 100.0f / 1048576.0f;  // 2^20 = 1048576
    data->temperature = (float)temperature_raw * 200.0f / 1048576.0f - 50.0f;
    
    // Validar faixas
    if (data->humidity < 0.0f) data->humidity = 0.0f;
    if (data->humidity > 100.0f) data->humidity = 100.0f;
    
    data->valid = true;
    return true;
}

// Determinar nível de conforto baseado em temperatura e umidade
const char* aht10_get_comfort_level(float temp, float humidity) {
    // Zona de conforto: 20-26°C e 40-60% umidade
    if (temp >= 20.0f && temp <= 26.0f && humidity >= 40.0f && humidity <= 60.0f) {
        return "😊 CONFORTÁVEL";
    }
    // Muito frio
    else if (temp < 15.0f) {
        return "🥶 MUITO FRIO";
    }
    // Frio
    else if (temp < 20.0f) {
        return "❄️ FRIO";
    }
    // Muito quente
    else if (temp > 30.0f) {
        return "🔥 MUITO QUENTE";
    }
    // Quente
    else if (temp > 26.0f) {
        return "🌡️ QUENTE";
    }
    // Muito seco
    else if (humidity < 30.0f) {
        return "🏜️ MUITO SECO";
    }
    // Seco
    else if (humidity < 40.0f) {
        return "🌵 SECO";
    }
    // Muito úmido
    else if (humidity > 70.0f) {
        return "💧 MUITO ÚMIDO";
    }
    // Úmido
    else if (humidity > 60.0f) {
        return "🌊 ÚMIDO";
    }
    // Caso padrão
    else {
        return "🤔 MODERADO";
    }
}
