#include <stdio.h>

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/semphr.h"
#include "http_client.h"
#include "led.h"
#include "nvs_flash.h"
#include "wifi.h"

#define CLEAR printf("\e[1;1H\e[2J")

EventGroupHandle_t s_wifi_event_group;
DadosTempo dadosTempo;

void menuInformacoes() {
    while (true) {
        CLEAR;
        if (!xEventGroupGetBits(s_wifi_event_group)) {
            printf("Wi-Fi Desconectado\n");
            printf("Aguardando conexão...\n");
        } else if (dadosTempo.tempAtual) {
            printf("%s, %s\n\n", dadosTempo.cidade, dadosTempo.pais);
            printf("Temperatura Atual: %.2f ºC\n", dadosTempo.tempAtual);
            printf("Sensação Térmica: %.2f ºC\n", dadosTempo.sensacaoTermica);
            printf("Temperatura Mínima: %.2f ºC\n", dadosTempo.tempMin);
            printf("Temperatura Máxima: %.2f ºC\n\n", dadosTempo.tempMax);
            printf("Umidade: %d%%\n\n", dadosTempo.umidade);
        } else {
            printf("Wi-Fi conectado\n");
            printf("Aguardando dados\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void RealizaHTTPRequest(void* params) {
    while (true) {
        if (xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY)) {
            ESP_LOGI("Main Task", "Realiza HTTP Request");
            xTaskCreate(&led_piscaUmaVez, "LED PISCA UMA VEZ", 2048, NULL, 1, NULL);
            xTaskCreate(&http_request, "HTTP CLIENT", 3072, NULL, 1, NULL);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_start();
    xTaskCreate(&menuInformacoes, "MENU INFORMAÇÕES", 3072, NULL, 1, NULL);
    xTaskCreate(&RealizaHTTPRequest, "Processa HTTP", 3072, NULL, 1, NULL);
}
