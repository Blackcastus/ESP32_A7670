#ifndef _APP_MAIN_H_
#define _APP_MAIN_H_

/* library system */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* config module sim GSM */
#define GSM_PWR_KEY (GPIO_NUM_2)
#define GSM_TXD (GPIO_NUM_5)
#define GSM_RXD (GPIO_NUM_17)
#define GSM_UART_PORT (UART_NUM1)
#define GSM_BAUD_RATE (115200)

/* void function task */
void GSM_Init();
void GSM_Write_CMD(const char *logName, const char *data);
void GSM_Read_Data(const char *logName);

#endif