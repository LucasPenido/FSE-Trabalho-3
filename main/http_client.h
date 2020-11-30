#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string.h>

#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/task.h"

typedef struct dadosTempo {
    float tempAtual;
    float sensacaoTermica;
    float tempMin;
    float tempMax;
    int umidade;
    char cidade[20];
    char pais[3];
} DadosTempo;

void http_request();
void https_request();

#endif
