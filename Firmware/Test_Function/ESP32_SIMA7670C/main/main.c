// #include "app_main.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* library RTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

/* library ESP */
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

/* config module sim GSM */
#define GSM_PWR_KEY (GPIO_NUM_2)
#define GSM_TXD (GPIO_NUM_5)
#define GSM_RXD (GPIO_NUM_17)
#define GSM_UART_PORT (UART_NUM_1)
#define GSM_BAUD_RATE (115200)
#define GSM_WAKEUP_PIN 2

/* task Handle RTOS */
TaskHandle_t Task_1 = NULL;
TaskHandle_t Task_2 = NULL;

/* static variable */
static const char *TAG = "SIM_A7670";
const int GSM_RX_BUF_SIZE = 1024;

/* void function task */
void GSM_Init();
int GSM_Write_CMD(const char *logName, const char *data);
void GSM_Read_Data(const char *logName,  uint8_t *data);
void Task_GSM_Write(void *arg);
void Task_GSM_Read(void *arg);


void app_main(void)
{
    /* init module sim A7670 communication with UART */
    GSM_Init();
    vTaskDelay(10000/portTICK_PERIOD_MS);

    /* create task rtos */
    xTaskCreate(&Task_GSM_Read, "GSM_Read_Data", 1024*2, NULL, 1, Task_2);
    xTaskCreate(&Task_GSM_Write, "GSM_Write_AT", 1024*2, NULL, 1, Task_1);
}

void Task_GSM_Write(void *arg)
{
    char GSM_AT[] = "AT\n";
    char GSM_CPIN[] = "AT+CPIN?+\n";
    while(1)
    {
        printf("%s", GSM_AT);
        uart_write_bytes(GSM_UART_PORT, &GSM_AT, strlen(GSM_AT));
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

void Task_GSM_Read(void *arg)
{
    static const char *GSM_Read_TAG  = "GSM_READ";
    esp_log_level_set(GSM_Read_TAG, ESP_LOG_INFO);
    uint8_t * data = (uint8_t*)malloc(GSM_RX_BUF_SIZE);
    while(1)
    {
        GSM_Read_Data(GSM_Read_TAG, data);
    }
    free(data);
}

void GSM_Init()
{
    /* config UART */
    const uart_config_t uart_config = {
        .baud_rate = GSM_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(GSM_UART_PORT, GSM_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(GSM_UART_PORT, &uart_config);
    uart_set_pin(GSM_UART_PORT, GSM_TXD, GSM_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "Init GSM finish");

    /* config GPIO WAKEUP */
    gpio_set_direction(GSM_WAKEUP_PIN, GPIO_MODE_OUTPUT);

    vTaskDelay(3000/portTICK_PERIOD_MS);

    gpio_set_level(GSM_WAKEUP_PIN, 1);
    vTaskDelay(5000/portTICK_PERIOD_MS);
    gpio_set_level(GSM_WAKEUP_PIN, 0);
    ESP_LOGI(TAG, "Turn on GSM");
}

int GSM_Write_CMD(const char *logName, const char *data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(GSM_UART_PORT, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

void GSM_Read_Data(const char *logName, uint8_t *data)
{
    const int rxBytes = uart_read_bytes(GSM_UART_PORT, data, GSM_RX_BUF_SIZE, 100 / portTICK_PERIOD_MS);
    if(rxBytes > 0)
    {
        data[rxBytes] = '\0';
        ESP_LOGI(logName, "Read %d bytes: %s", rxBytes, data);
    }
}