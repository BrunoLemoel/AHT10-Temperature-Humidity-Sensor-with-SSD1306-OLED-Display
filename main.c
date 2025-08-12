#include <stdio.h>
#include "pico/stdlib.h"
#include "aht10.h"
#include "display.h"

// Variáveis globais
static bool system_initialized = false;

int main() {
    stdio_init_all();
    
    printf("===============================================\n");
    printf("SISTEMA SENSOR AHT10 - TEMPERATURA & UMIDADE\n");
    printf("Raspberry Pi Pico W - EmbarcaTech 2025\n");
    printf("===============================================\n");
    printf("Configuração:\n");
    printf("  - Sensor: AHT10 (I2C0, GPIO0/1)\n");
    printf("  - Display: LCD 320x240 (I2C1, GPIO14/15)\n");
    printf("===============================================\n");
    
    // Aguardar estabilização
    sleep_ms(1000);
    
    // Inicializar display primeiro
    printf("\n--- INICIALIZANDO DISPLAY ---\n");
    bool display_ok = display_init();
    if (display_ok) {
        display_show_startup_screen();
    }
    
    // Inicializar sensor AHT10
    printf("\n--- INICIALIZANDO SENSOR AHT10 ---\n");
    bool sensor_ok = aht10_init();
    
    if (!sensor_ok) {
        printf("\n❌ FALHA NA INICIALIZAÇÃO DO SENSOR!\n");
        printf("Verifique as conexões e reinicie o programa.\n");
        
        if (display_ok) {
            display_show_error_screen("Sensor AHT10 nao encontrado");
        }
        
        // Loop de espera e tentativas de reconexão
        while (true) {
            printf("\n⏳ Tentando detectar sensor novamente em 5s...\n");
            sleep_ms(5000);
            
            sensor_ok = aht10_detect();
            if (sensor_ok) {
                printf("\n🔄 SENSOR DETECTADO! Reinicializando...\n");
                sensor_ok = aht10_init();
                if (sensor_ok) {
                    printf("✅ SENSOR RECONECTADO!\n");
                    break;
                }
            }
        }
    }
    
    system_initialized = true;
    printf("\n� SISTEMA PRONTO! Iniciando leituras...\n");
    printf("===============================================\n");
    printf("Formato: Temp | Umidade | Status\n");
    printf("===============================================\n");
    
    // Contador para controle de display
    uint32_t display_update_counter = 0;
    const uint32_t DISPLAY_UPDATE_INTERVAL = 5; // Atualizar display a cada 5 leituras
    
    // Loop principal de leitura
    while (true) {
        aht10_data_t sensor_data;
        
        // Ler dados do sensor
        bool read_success = aht10_read_data(&sensor_data);
        
        if (read_success && sensor_data.valid) {
            // Determinar nível de conforto
            const char* comfort = aht10_get_comfort_level(sensor_data.temperature, sensor_data.humidity);
            
            // Imprimir no terminal
            printf("%.1f°C | %.1f%% | %s\n", 
                   sensor_data.temperature, 
                   sensor_data.humidity, 
                   comfort);
            
            // Atualizar display periodicamente para evitar flickering
            if (display_ok && (display_update_counter % DISPLAY_UPDATE_INTERVAL == 0)) {
                display_update_sensor_data(sensor_data);
            }
            
            display_update_counter++;
        } else {
            printf("❌ Erro na leitura do sensor\n");
            
            // Mostrar erro no display ocasionalmente
            if (display_ok && (display_update_counter % DISPLAY_UPDATE_INTERVAL == 0)) {
                sensor_data.valid = false;
                display_update_sensor_data(sensor_data);
            }
            
            display_update_counter++;
        }
        
        // Aguardar antes da próxima leitura
        sleep_ms(2000); // Leitura a cada 2 segundos
    }
    
    return 0;
}
