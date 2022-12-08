/*
 * fsm.c
 *
 *  Created on: Nov 20, 2022
 *      Author: PC
 */

#include "fsm.h"

#define IDLE 0
#define CMD_WAITING 1
#define CMD_WAITING_O 2
#define CMD_WAITING_R 3
#define CMD_WAITING_RS 4
#define CMD_CONFIRM 5

enum CMD {NON, RST, OK};

int cmdParserStatus = IDLE;
int ADC1_value = -1;
int ADC2_value = -1;
enum CMD cmd = NON;
int index = 0;

uint8_t buffer[MAX_BUFFER_SIZE];
int index_buffer;
int receive_flag;
uint8_t temp;
int RST_flag;
int OK_flag;

void command_parser_fsm(UART_HandleTypeDef* huart2)
{
	char writeStr[10];
	HAL_UART_Transmit(huart2, writeStr, sprintf(writeStr, "Index %d\r\n", index), 1000);
	HAL_UART_Transmit(huart2, writeStr, sprintf(writeStr, "Receiving %c\r\n", buffer[index]), 1000);
	switch (cmdParserStatus)
	{
	case IDLE:
		if (buffer[index] == '!')
		{
			cmdParserStatus = CMD_WAITING;
		}
		index++;
		if (receive_flag == 0)
		{
			index_buffer = 0;
			index = 0;
		}
		break;
	case CMD_WAITING:
		if (buffer[index] == 'O')
		{
			cmdParserStatus = CMD_WAITING_O;
		}
		else if (buffer[index] == 'R')
		{
			cmdParserStatus = CMD_WAITING_R;
		}
		else cmdParserStatus = IDLE;
		index++;
		if (receive_flag == 0)
		{
			index_buffer = 0;
			index = 0;
		}
		break;
//-----------------------------------------------------
	case CMD_WAITING_O:
		if (buffer[index] == 'K')
		{
			cmdParserStatus = CMD_CONFIRM;
			cmd = OK;
		}
		else cmdParserStatus = IDLE;
		index++;
		if (receive_flag == 0)
		{
			index_buffer = 0;
			index = 0;
		}
		break;
//-----------------------------------------------------
	case CMD_WAITING_R:
		if (buffer[index] == 'S')
		{
			cmdParserStatus = CMD_WAITING_RS;
		}
		else cmdParserStatus = IDLE;
		index++;
		if (receive_flag == 0)
		{
			index_buffer = 0;
			index = 0;
		}
		break;
	case CMD_WAITING_RS:
		if (buffer[index] == 'T')
		{
			cmd = RST;
			cmdParserStatus = CMD_CONFIRM;
		}
		else cmdParserStatus = IDLE;
		index++;
		if (receive_flag == 0)
		{
			index_buffer = 0;
			index = 0;
		}
		break;
//-----------------------------------------------------
	case CMD_CONFIRM:
			if (buffer[index] == '#')
			{
				if (cmd == OK)
				{
					OK_flag = 1;
				}
				if (cmd == RST)
				{
					setTimer(0, 10);
					RST_flag = 1;
				}
				cmd = NON;
			}
			cmdParserStatus = IDLE;
			index++;
			if (receive_flag == 0)
			{
				index_buffer = 0;
				index = 0;
			}
			break;
	default:
		break;
	}
}

void uart_communication_fsm(UART_HandleTypeDef* huart2, ADC_HandleTypeDef* hadc1, ADC_HandleTypeDef* hadc2)
{
	char writeStr[50];
	if (RST_flag == 1)
	{

		//HAL_UART_Transmit(huart2, writeStr, sprintf(writeStr, "RST\n"), 1000);
		//ADC_value = HAL_ADC_GetValue(&hadc1);
			//HAL_UART_Transmit(huart2, writeStr, sprintf(writeStr, "Different\n"), 1000);
		if (timer_timeout(0))
		{
			ADC1_value = HAL_ADC_GetValue(hadc1);
			HAL_UART_Transmit(huart2, writeStr, sprintf(writeStr, "!ADC1=%d#\r\n", ADC1_value), 1000);
			ADC2_value = HAL_ADC_GetValue(hadc2);
			HAL_UART_Transmit(huart2, writeStr, sprintf(writeStr, "!ADC2=%d#\r\n", ADC2_value), 1000);
			setTimer(0, 3000);
		}

		//RST_flag = 0;
		//ADC_value = HAL_ADC_GetValue(&hadc1);
	}
	if (OK_flag == 1)
	{
		OK_flag = 0;
		RST_flag = 0;
		ADC1_value = -1;
		ADC2_value = -1;

	}
}
