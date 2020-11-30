#include "http_client.h"

#define TAG "HTTP"
#define KEY_OPENWHEATHER CONFIG_KEY_OPENWEATHER
#define KEY_IPSTACK CONFIG_KEY_IPSTACK

extern DadosTempo dadosTempo;
char *requestResponse;
float latitude, longitude;

void trataResposta(int tamanhoDado, void *dado) {
    char *temp;

    temp = (char *)malloc(tamanhoDado + 1);
    strncpy(temp, (char *)dado, tamanhoDado);
    temp[tamanhoDado] = '\0';

    requestResponse = (char *)realloc(requestResponse, strlen(requestResponse) + strlen(temp) + 1);
    strcat(requestResponse, temp);

    free(temp);
}

esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char *)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            trataResposta(evt->data_len, evt->data);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void ipstackRequest() {
    requestResponse = (char *)malloc(sizeof(char));
    requestResponse[0] = '\0';
    char ipstackQueryFields[] = "&fields=latitude,longitude";
    char ipstackUrl[100] = "http://api.ipstack.com/check?access_key=";

    strcat(ipstackUrl, CONFIG_KEY_IPSTACK);
    strcat(ipstackUrl, ipstackQueryFields);

    esp_http_client_config_t ipstackConfig = {
        .url = ipstackUrl,
        .event_handler = _http_event_handle,
    };

    esp_http_client_handle_t ipstackClient = esp_http_client_init(&ipstackConfig);
    esp_err_t err = esp_http_client_perform(ipstackClient);

    esp_http_client_cleanup(ipstackClient);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ERRO AO CONECTAR 1");
        vTaskDelete(NULL);
    }
}

void openweatherRequest() {
    requestResponse = (char *)malloc(sizeof(char));
    requestResponse[0] = '\0';

    int bufferSize = 95;
    char openweatherUrl[140] = "http://api.openweathermap.org/data/2.5/weather";
    char queryBuffer[bufferSize];

    snprintf(queryBuffer, bufferSize,
             "?lon=%f&lat=%f&appid=%s&units=metric&lang=pt_br", longitude, latitude, CONFIG_KEY_OPENWEATHER);
    strcat(openweatherUrl, queryBuffer);

    esp_http_client_config_t openweatherConfig = {
        .url = openweatherUrl,
        .event_handler = _http_event_handle,
    };
    esp_http_client_handle_t openweatherClient = esp_http_client_init(&openweatherConfig);
    esp_err_t err = esp_http_client_perform(openweatherClient);

    esp_http_client_cleanup(openweatherClient);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ERRO AO CONECTAR 2");
        vTaskDelete(NULL);
    }
}

void http_request() {
    ipstackRequest();

    cJSON *ipstackJson = cJSON_Parse(requestResponse);
    longitude = cJSON_GetObjectItem(ipstackJson, "longitude")->valuedouble;
    latitude = cJSON_GetObjectItem(ipstackJson, "latitude")->valuedouble;

    free(requestResponse);

    openweatherRequest();

    cJSON *openweatherJson = cJSON_Parse(requestResponse);
    cJSON *dados = cJSON_GetObjectItem(openweatherJson, "main");
    cJSON *sys = cJSON_GetObjectItem(openweatherJson, "sys");

    strcpy(dadosTempo.cidade, cJSON_GetObjectItem(openweatherJson, "name")->valuestring);
    strcpy(dadosTempo.pais, cJSON_GetObjectItem(sys, "country")->valuestring);
    dadosTempo.tempAtual = cJSON_GetObjectItem(dados, "temp")->valuedouble;
    dadosTempo.tempMin = cJSON_GetObjectItem(dados, "temp_min")->valuedouble;
    dadosTempo.tempMax = cJSON_GetObjectItem(dados, "temp_max")->valuedouble;
    dadosTempo.sensacaoTermica = cJSON_GetObjectItem(dados, "feels_like")->valuedouble;
    dadosTempo.umidade = cJSON_GetObjectItem(dados, "humidity")->valueint;

    vTaskDelete(NULL);
}
