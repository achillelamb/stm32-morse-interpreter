/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <morse.h>
#include <retarget.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define dbg(...) printf(__VA_ARGS__)
#else
#define dbg(...)                                                                                   \
    do {                                                                                           \
    } while (0)
#endif
#define MIN_PRESS_THRESHOLD 50   // milliseconds
#define LONG_PRESS_THRESHOLD 300 // milliseconds
#define MAX_PRESS_THRESHOLD 1500 // milliseconds
#define HANDLE_MEMORY_ERROR(ptr)                                                                   \
    do {                                                                                           \
        if (!ptr) {                                                                                \
            goto error;                                                                            \
        }                                                                                          \
    } while (0)

const enum morse_char_t NULL_CHAR[] = {End, End, End, End, End, End, End, End};

UART_HandleTypeDef huart2;

uint32_t press_time = 0;      // Store the press time (milliseconds)
uint32_t release_time = 0;    // Store the release time (milliseconds)
uint32_t button_duration = 0; // Store the button press duration (milliseconds)

enum morse_char_t *morse_c = NULL;
uint8_t morse_index = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/*
 * @brief Checks that the USER BUTTON has not been pressed for at least `timeout` milliseconds
 *
 * @retval 0 if button was pressed less than `timeout` ms, gt 0 if the button was pressed more than
 * `timeout` ms ago
 */
static inline int not_pressed_for_al(uint32_t timeout) {
    return press_time < release_time && ((HAL_GetTick() - release_time) > timeout);
}

void hw_initialization() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* MCU Configuration----------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */
    /* Enables retarget of standard I/O over the USART2 */
    RetargetInit(&huart2);
    /* USER CODE END 2 */

    /* GPIOA, GPIOB and GPIOC Configuration----------------------------------------------*/
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x1, 0x0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

int main(void) {
    hw_initialization();

    MorseNode *root = initialize_morse_tree();
    HANDLE_MEMORY_ERROR(root);

    morse_c = (enum morse_char_t *)malloc(MORSE_CHAR_SIZE);
    HANDLE_MEMORY_ERROR(morse_c);
    nullate(morse_c);
    enum morse_char_t *prev_state = malloc(MORSE_CHAR_SIZE);
    HANDLE_MEMORY_ERROR(prev_state);
    nullate(prev_state);

    while (1) {
        HAL_Delay(100);
        if (not_pressed_for_al(1500) && !morse_eq(morse_c, NULL_CHAR)) {
            if (morse_eq(morse_c, prev_state)) {
                memcpy(prev_state, morse_c, MORSE_CHAR_SIZE);
            } else {
                char mchar = decode_morse(root, morse_c);
                dbg("The decoded morse is: %s\n\r",
                    mchar ? (char[]){mchar, 0} /* string composed by decoded char + null term */
                          : "INVALID");
                printf("%c", mchar);
                nullate(morse_c);
                nullate(prev_state);
                morse_index = 0;
            }
        }
    }

error:
    HAL_NVIC_SystemReset();
}

enum PressLength get_press_length(uint32_t duration) {
    if (duration > MIN_PRESS_THRESHOLD && duration <= LONG_PRESS_THRESHOLD)
        return Short;
    else if (duration > LONG_PRESS_THRESHOLD && duration < MAX_PRESS_THRESHOLD)
        return Long;
    return Unknown;
}

void determine_input_morse_symbol() {
    release_time = HAL_GetTick();
    button_duration = release_time - press_time;
    dbg("Press duration: %lu\n\r", button_duration);
    switch (get_press_length(button_duration)) {
    case Short:
        dbg(".");
        morse_c[morse_index++] = Dot;
        break;
    case Long:
        dbg("-");
        morse_c[morse_index++] = Dash;
        break;
    default:
        break;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_13) { // Check if the interrupt was triggered by pin 13 (user button)
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
            dbg("RELEASE\n\r");
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            determine_input_morse_symbol();
        } else {
            dbg("PRESS\n\r");
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            press_time = HAL_GetTick();
        }
    }
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : B1_Pin */
    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : LD2_Pin */
    GPIO_InitStruct.Pin = LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source dash number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  dash: assert_param error dash source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t dash) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and dash number,
       ex: printf("Wrong parameters value: file %s on dash %d\r\n", file, dash) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
